#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <errno.h>

#include <map>
#include <vector>
#include <memory>
#include <cstring>
#include <algorithm>

#include "../common/protocol.h"
#include "../common/messages.h"
#include "../common/net.hpp"
#include "client.h"
#include "game.h"

#define MAX_EVENTS 128

// --- Global Server State ---
std::map<int, std::shared_ptr<Client>> clients;
std::vector<Game> games;

// --- Forward Declarations ---
void handle_login(std::shared_ptr<Client> client, MsgLoginReq *msg);
void handle_create_game(std::shared_ptr<Client> client);
void disconnect_client(int cfd, int epoll_fd);
void process_message(std::shared_ptr<Client> client, MsgHeader& hdr, char* payload);
void handle_client_data(std::shared_ptr<Client> client, int epoll_fd);
void broadcast_lobby_state();
void send_lobby_state(std::shared_ptr<Client> client);

static void set_non_blocking(int fd) {
    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK);
}

int main() {
    int sfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sfd < 0) {
        perror("socket");
        return 1;
    }

    int opt = 1;
    setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in sa{};
    sa.sin_family = AF_INET;
    sa.sin_port = htons(12345);
    sa.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (bind(sfd, (sockaddr*)&sa, sizeof(sa)) < 0) {
        perror("bind");
        close(sfd);
        return 1;
    }
	if (listen(sfd, SOMAXCONN)){
		perror("listen");
		close(sfd);
		return 1;
	}
    printf("Serwer nasłuchuje na 127.0.0.1:12345\n");
    set_non_blocking(sfd);

    int epoll_fd = epoll_create1(0);
    if (epoll_fd < 0) {
        perror("epoll_create1");
        close(sfd);
        close(epoll_fd);
        return 1;
    }

    epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = sfd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sfd, &event) < 0) {
        perror("epoll_ctl ADD sfd");
        close(sfd);
        close(epoll_fd);
        return 1;
    }

    epoll_event events[MAX_EVENTS];

    while (true) {
        int n_events = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        if (n_events < 0) {
            perror("epoll_wait");
            continue;
        }

        for (int i = 0; i < n_events; ++i) {
            if (events[i].data.fd == sfd) {
                // New connection
                while (true) {
                    int cfd = accept(sfd, nullptr, nullptr);
                    if (cfd < 0) {
                        if (errno == EAGAIN || errno == EWOULDBLOCK) break; // No more incoming connections
                        perror("accept");
                        break;
                    }

                    set_non_blocking(cfd);
                    auto client = std::make_shared<Client>();
                    client->fd = cfd;
                    client->active = 1;
                    client->state = STATE_CONNECTED;
                    client->game_id = -1;
                    memset(client->nick, 0, sizeof(client->nick));

                    clients[cfd] = client;
                    
                    event.events = EPOLLIN;
                    event.data.fd = cfd;
                    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, cfd, &event) < 0) {
                        perror("epoll_ctl ADD cfd");
                        disconnect_client(cfd, epoll_fd);
                        continue;
                    }
                    printf("Klient połączony (fd=%d)\n", cfd);
                }
            } else {
                // Data from client
                int cfd = events[i].data.fd;
                if (events[i].events & (EPOLLERR | EPOLLHUP)) {
                    disconnect_client(cfd, epoll_fd);
                    continue;
                }
                
                auto it = clients.find(cfd);
                if (it != clients.end()) {
                    handle_client_data(it->second, epoll_fd);
                }
            }
        }
    }

    close(sfd);
    close(epoll_fd);
    return 0;
}


void handle_client_data(std::shared_ptr<Client> client, int epoll_fd) {
    char read_buffer[1024];
    int cfd = client->fd;

    ssize_t bytes_read = recv(cfd, read_buffer, sizeof(read_buffer), 0);

    if (bytes_read == 0) {
        printf("Klient (%s) rozłączony (EOF)\n", client->nick);
        disconnect_client(cfd, epoll_fd);
        return;
    }
    if (bytes_read < 0) {
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
            perror("recv");
            disconnect_client(cfd, epoll_fd);
        }
        return;
    }

    client->buffer.insert(client->buffer.end(), read_buffer, read_buffer + bytes_read);

    while (true) {
        if (client->buffer.size() < sizeof(MsgHeader)) {
            break; // Not enough data for a header
        }

        MsgHeader hdr;
        memcpy(&hdr, client->buffer.data(), sizeof(hdr));

        if (client->buffer.size() < sizeof(MsgHeader) + hdr.length) {
            break; // Not enough data for the full payload
        }

        // We have a full message
        std::vector<char> payload(hdr.length);
        if (hdr.length > 0) {
            memcpy(payload.data(), client->buffer.data() + sizeof(MsgHeader), hdr.length);
        }

        process_message(client, hdr, payload.data());

        // Remove the processed message from the buffer
        client->buffer.erase(client->buffer.begin(), client->buffer.begin() + sizeof(MsgHeader) + hdr.length);
    }
}

void process_message(std::shared_ptr<Client> client, MsgHeader& hdr, char* payload) {
    if (client->state < STATE_LOGGING_IN && hdr.type != MSG_LOGIN_REQ) {
        printf("Nieprawidłowa wiadomość przed logowaniem (fd=%d)\n", client->fd);
        return;
    }

    switch (hdr.type) {
        case MSG_LOGIN_REQ:
            handle_login(client, (MsgLoginReq*)payload);
            break;
		case MSG_CREATE_GAME_REQ:
			handle_create_game(client);
			break;

        default:
            printf("Nieznany typ wiadomości %d (fd=%d)\n", hdr.type, client->fd);
            break;
    }
}

void handle_login(std::shared_ptr<Client> client, MsgLoginReq *msg) {
    client->state = STATE_LOGGING_IN;

    char received_nick[MAX_NICK_LEN];
    strncpy(received_nick, msg->nick, MAX_NICK_LEN - 1);
    received_nick[MAX_NICK_LEN - 1] = '\0';

    printf("Logowanie klienta (fd=%d) nick=%s\n", client->fd, received_nick);
    
    bool nick_taken = false;
    for (auto const& [fd, other] : clients) {
        if (other != client &&
            other->active &&
            strcmp(other->nick, received_nick) == 0) {
            nick_taken = true;
            break;
        }
    }

    if (nick_taken) {
        send_msg(client->fd, MSG_LOGIN_TAKEN, nullptr, 0);
        client->state = STATE_CONNECTED;
        return;
    }
    
    strcpy(client->nick, received_nick);
    client->state = STATE_LOBBY;
    send_msg(client->fd, MSG_LOGIN_OK, nullptr, 0);
    send_lobby_state(client);

    printf("Klient zalogowany: %s (fd=%d)\n", client->nick, client->fd);
}


void disconnect_client(int cfd, int epoll_fd) {
    if (clients.find(cfd) == clients.end()) {
        return; // Already disconnected
    }
    
    if (epoll_fd > 0) {
        epoll_ctl(epoll_fd, EPOLL_CTL_DEL, cfd, nullptr);
    }
    
    close(cfd);
    clients.erase(cfd);

    printf("Klient usunięty (fd=%d)\n", cfd);
}

void handle_create_game(std::shared_ptr<Client> client) {
    if (client->state != STATE_LOBBY) {
        printf("Klient %s nie jest w lobby – nie może stworzyć gry\n", client->nick);
        return;
    }

    Game game;
    memset(&game, 0, sizeof(Game));

    // --- ID gry ---
    game.id = games.empty() ? 1 : games.back().id + 1;

    // --- Owner ---
    strncpy(game.owner, client->nick, sizeof(game.owner) - 1);

    // --- Gracze ---
    game.players[0] = client->fd;   // lub client->id jeśli dodasz ID
    game.player_count = 1;

    // --- Stan gry ---
    game.started = 0;

    // --- Reset guessed ---
    memset(game.guessed, 0, sizeof(game.guessed));

    // --- Słowo (na razie puste) ---
    memset(game.word, 0, sizeof(game.word));

    games.push_back(game);
	broadcast_lobby_state();

    client->game_id = game.id;

    printf(
        "Utworzono grę id=%d owner=%s\n",
        game.id,
        game.owner
    );

    // --- Powiadom twórcę ---
    send_msg(client->fd, MSG_CREATE_GAME_OK, nullptr, 0);

    // --- TODO: broadcast nowego lobby state do wszystkich w lobby ---
}

void send_lobby_state(std::shared_ptr<Client> client) {
    MsgLobbyState msg;
    memset(&msg, 0, sizeof(msg));

    msg.games_count = 0;

    for (const auto& game : games) {
        if (msg.games_count >= MAX_GAMES)
            break;

        msg.games[msg.games_count].game_id = game.id;
        msg.games[msg.games_count].players_count = game.player_count;
        msg.games_count++;
    }

    send_msg(client->fd, MSG_LOBBY_STATE, &msg, sizeof(msg));
}

void broadcast_lobby_state() {
    for (auto& [fd, other] : clients) {
        if (other->state == STATE_LOBBY) {
            send_lobby_state(other);
        }
    }
}