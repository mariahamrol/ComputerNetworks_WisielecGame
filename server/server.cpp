#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <thread>
#include <mutex>
#include <vector>
#include <string.h>
#include <algorithm>

#include "../common/protocol.h"
#include "../common/messages.h"
#include "../common/net.hpp"
#include "client.h"
#include "game.h"

#define MAX_CLIENTS 64

// --- Global Server State ---
Client clients[MAX_CLIENTS];
std::vector<Game> games;
std::mutex clients_mutex;
std::mutex games_mutex;

// --- Forward Declarations ---
void handle_client(int client_idx);
void handle_login(int client_idx, MsgLoginReq *msg);
void disconnect_client(int client_idx);

void init_server() {
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        clients[i].active = 0;
        clients[i].fd = -1;
    }
}

int main(int argc, char **argv) {
    init_server();

    int sfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sfd < 0) {
        perror("socket");
        return 1;
    }
    int opt = 1;
    setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in sa{};
    sa.sin_family = AF_INET;
    sa.sin_port = htons(12345);
    sa.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (bind(sfd, (sockaddr*)&sa, sizeof(sa)) < 0) {
        perror("bind");
        return 1;
    }
    listen(sfd, MAX_CLIENTS);
    printf("Serwer nasłuchuje na 127.0.0.1:12345\n");

    while (true) {
        int cfd = accept(sfd, NULL, NULL);
        if (cfd < 0) {
            perror("accept");
            continue;
        }

        std::lock_guard<std::mutex> lock(clients_mutex);
        int client_idx = -1;
        for (int i = 0; i < MAX_CLIENTS; ++i) {
            if (!clients[i].active) {
                client_idx = i;
                break;
            }
        }

        if (client_idx == -1) {
            printf("Serwer jest pełny. Odrzucono połączenie.\n");
            // Można wysłać wiadomość o błędzie, ale zamykamy od razu
            close(cfd);
            continue;
        }

        clients[client_idx].fd = cfd;
        clients[client_idx].active = 1;
        clients[client_idx].state = STATE_CONNECTED;
        clients[client_idx].game_id = -1;
        memset(clients[client_idx].nick, 0, sizeof(clients[client_idx].nick));

        printf("Klient połączony. Przypisano indeks %d\n", client_idx);
        std::thread(handle_client, client_idx).detach();
    }

    close(sfd);
    return 0;
}

void handle_client(int client_idx) {
    MsgHeader hdr;
    char payload[1024];

    int fd = clients[client_idx].fd;

    while (true) {
        int r = recv_msg(fd, hdr, payload, sizeof(payload));
        if (r <= 0) {
            printf("Klient (indeks %d) rozłączony.\n", client_idx);
            disconnect_client(client_idx);
            return;
        }
        
        // Tymczasowo blokujemy obsługę innych wiadomości, dopóki klient się nie zaloguje
        if (clients[client_idx].state < STATE_LOGGING_IN) {
            if(hdr.type != MSG_LOGIN_REQ) {
                 printf("Klient (indeks %d) wysłał nieprawidłową wiadomość przed logowaniem.\n", client_idx);
                 continue;
            }
        }

        switch (hdr.type) {
            case MSG_LOGIN_REQ:
                handle_login(client_idx, (MsgLoginReq*)payload);
                break;

            // TODO: Obsługa kolejnych typów wiadomości
            // case MSG_CREATE_GAME_REQ:
            //     handle_create_game(client_idx);
            //     break;
            //
            // case MSG_JOIN_GAME_REQ:
            //     handle_join_game(client_idx, (MsgJoinGameReq*)payload);
            //     break;

            default:
                printf("Otrzymano nieznany typ wiadomości: %d od klienta %d\n", hdr.type, client_idx);
                // Opcjonalnie można odesłać błąd
                // send_msg(fd, MSG_ERROR, NULL, 0);
                break;
        }
    }
}

void handle_login(int client_idx, MsgLoginReq *msg) {
    clients[client_idx].state = STATE_LOGGING_IN;
    char received_nick[MAX_NICK_LEN];
    strncpy(received_nick, msg->nick, MAX_NICK_LEN -1);
    received_nick[MAX_NICK_LEN - 1] = '\0';

    printf("Klient (indeks %d) próbuje się zalogować z nickiem: %s\n", client_idx, received_nick);

    std::lock_guard<std::mutex> lock(clients_mutex);

    bool nick_taken = false;
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (clients[i].active && i != client_idx && strcmp(clients[i].nick, received_nick) == 0) {
            nick_taken = true;
            break;
        }
    }

    if (nick_taken) {
        printf("Nick '%s' jest już zajęty. Odrzucono logowanie klienta %d.\n", received_nick, client_idx);
        send_msg(clients[client_idx].fd, MSG_LOGIN_TAKEN, NULL, 0);
        clients[client_idx].state = STATE_CONNECTED;
    } else {
        strcpy(clients[client_idx].nick, received_nick);
        clients[client_idx].state = STATE_LOBBY;
        printf("Klient %d zalogowany jako '%s'.\n", client_idx, clients[client_idx].nick);
        send_msg(clients[client_idx].fd, MSG_LOGIN_OK, NULL, 0);
    }
}

void disconnect_client(int client_idx) {
    std::lock_guard<std::mutex> lock(clients_mutex);
    if (clients[client_idx].active) {
        close(clients[client_idx].fd);
        clients[client_idx].active = 0;
        clients[client_idx].fd = -1;
        printf("Zasoby dla klienta %d zostały zwolnione.\n", client_idx);

        // TODO: Dodatkowa logika przy rozłączeniu, np. usunięcie z gry
    }
}