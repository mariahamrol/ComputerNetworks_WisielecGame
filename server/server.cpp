#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>

#include <map>
#include <vector>
#include <unordered_map>
#include <memory>
#include <cstring>
#include <algorithm>
#include <random>

#include "../include/protocol.h"
#include "../include/messages.h"
#include "../include/net.hpp"
#include "../include/wordlist.h"
#include "../client/client.h"
#include "game.h"

#define MAX_EVENTS 128

const char* admin_pwd = "3edcvfr4";

static uint32_t next_game_id = 1;
volatile sig_atomic_t server_running = 1;

// --- Global Server State ---
std::map<int, std::shared_ptr<Client>> clients;
std::unordered_map<uint32_t, Game> games;

// --- Forward Declarations ---
void handle_login(std::shared_ptr<Client> client, MsgLoginReq *msg);
void handle_admin_login(std::shared_ptr<Client> client, MsgAdminLoginReq *msg);
void handle_admin_list_games(std::shared_ptr<Client> client);
void handle_admin_list_users(std::shared_ptr<Client> client);
void handle_admin_game_details(std::shared_ptr<Client> client, MsgGameIdReq* msg);
void handle_admin_terminate_game(std::shared_ptr<Client> client, MsgGameIdReq* msg);
void handle_create_room(std::shared_ptr<Client> client);
void disconnect_client(int cfd, int epoll_fd);
void process_message(std::shared_ptr<Client> client, MsgHeader& hdr, char* payload);
void handle_client_data(std::shared_ptr<Client> client, int epoll_fd);
void broadcast_lobby_state();
void send_lobby_state(std::shared_ptr<Client> client);
void handle_join_room(std::shared_ptr<Client> client,  MsgGameIdReq *msg);
void handle_start_game(std::shared_ptr<Client> client,  MsgGameIdReq *msg);
void create_game_word(Game& game);
void handle_guess_letter(std::shared_ptr<Client> client,  MsgGuessLetterReq *msg);
void brodcast_game_state(Game& game);
void delete_player_from_game(std::shared_ptr<Client> client);
void send_game_results(Game& game);
void delete_game(uint32_t game_id);
void start_new_turn(Game& game);
void broadcast_to_game(Game& game, uint16_t msg_type, const void* payload = nullptr, uint16_t length = 0);
void user_exit_game(std::shared_ptr<Client> client);
void user_exit_room(std::shared_ptr<Client> client);
void handle_shutdown(int sig);
void graceful_shutdown();
Game* find_game_by_id(uint32_t game_id);

static void set_non_blocking(int fd) {
    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK);
}

int main() {
	signal(SIGPIPE, SIG_IGN);
	signal(SIGINT, handle_shutdown); 
	signal(SIGTERM, handle_shutdown); 
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
	sa.sin_addr.s_addr = INADDR_ANY;
    // sa.sin_addr.s_addr = inet_addr("127.0.0.1");

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
    printf("Serwer nasłuchuje na wszystkich ip na porcie 12345\n");
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

    while (server_running) {
        int n_events = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        if (n_events < 0) {
            perror("epoll_wait");
            continue;
        }

        for (int i = 0; i < n_events; ++i) {
            if (events[i].data.fd == sfd) {
                // New connection
                while (server_running) {
                    int cfd = accept(sfd, nullptr, nullptr);
                    if (cfd < 0) {
                        if (errno == EAGAIN || errno == EWOULDBLOCK) break; // No more incoming connections
                        perror("accept");
                        break;
                    }

                    set_non_blocking(cfd);
                    auto client = std::make_shared<Client>();
                    client->fd = cfd;
                    client->is_active = 1;
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

	graceful_shutdown();
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

    while (server_running) {
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
		case MSG_ADMIN_LIST_GAMES_REQ:
			handle_admin_list_games(client);
			break;
		case MSG_ADMIN_LIST_USERS_REQ:
			handle_admin_list_users(client);
			break;
		case MSG_ADMIN_GAME_DETAILS_REQ:
			handle_admin_game_details(client, (MsgGameIdReq*)payload);
			break;
		case MSG_ADMIN_LOGIN_REQ:
			handle_admin_login(client, (MsgAdminLoginReq*)payload);
			break;
		case MSG_ADMIN_TERMINATE_GAME:
			handle_admin_terminate_game(client, (MsgGameIdReq*)payload);
			break;
		case MSG_CREATE_ROOM_REQ:
			handle_create_room(client);
			break;
		case MSG_JOIN_ROOM_REQ:
			handle_join_room(client, (MsgGameIdReq*)payload);
			break;
		case MSG_START_GAME_REQ:
			handle_start_game(client, (MsgGameIdReq*)payload);
			break;
		case  MSG_GUESS_LETTER_REQ:
			handle_guess_letter(client, (MsgGuessLetterReq*)payload);
			break;
		case MSG_EXIT_GAME_REQ:
			user_exit_game(client);
			break;
		case MSG_EXIT_ROOM_REQ:
		 	user_exit_room(client);
			break;
        default:
            printf("Nieznany typ wiadomości %d (fd=%d)\n", hdr.type, client->fd);
            break;
    }
}

void handle_guess_letter(std::shared_ptr<Client> client,  MsgGuessLetterReq *msg) {
	if (client->state != STATE_IN_GAME || !client->is_active || client->lives <= 0){
		printf("Nieautoryzowana próba odgadnięcia litery przez %s, state=%d, is_active=%d, lives=%d\n", client->nick, client->state, client->is_active, client->lives);
		if(send_msg(client->fd, MSG_GUESS_LETTER_FAIL, nullptr, 0) != 0) {
			perror("send_msg GUESS_LETTER_FAIL");
		}
		return;
	}
	Game* game = find_game_by_id(client->game_id);
	if (game == nullptr || !game->active) {
		printf("Nieprawidłowa próba odgadnięcia litery przez %s w grze id=%d\n", client->nick, client->game_id);
		if(send_msg(client->fd, MSG_GUESS_LETTER_FAIL, nullptr, 0) != 0) {
			perror("send_msg GUESS_LETTER_FAIL");
		}
		return;
	}
	const char letter = msg->letter; 
	printf("Gracz %s próbuje odgadnąć literę '%c' w grze id=%d\n", client->nick, letter, client->game_id);
	bool already_guessed = false;
	for (int i = 0; i < ALPHABET_SIZE; i++) {
		if (client->guessed_letters[i] == letter || game->guessed_letters[i] == letter) {
			already_guessed = true;
			break;
		}
	}
	if (already_guessed) {
		printf("Litera %c już odgadnięta przez %s w grze id=%d\n", letter, client->nick, client->game_id);
		if(send_msg(client->fd, MSG_GUESS_LETTER_FAIL, nullptr, 0) != 0) {
			perror("send_msg GUESS_LETTER_FAIL");
		}
		return;
	}

	uint8_t correct = 0;
	uint8_t count = 0;
	for (int i = 0; i < game->word_length; ++i) {
		if (game->word[i] == letter) {
			game->word_guessed[i] = letter;
			count++;
			correct = 1;
		}
	}

	if (correct == 0) {
		client->lives--;
		if (client->lives == 0) {
			client->is_active = 0;
			if (send_msg(client->fd, MSG_PLAYER_ELIMINATED, nullptr, 0) != 0) {
				perror("send_msg PLAYER_ELIMINATED");
			}
			printf("Gracz %s stracił wszystkie życia i jest nieaktywny w grze id=%d\n", client->nick, client->game_id);
		}
	}

	if (count > 0) {
		client->points += count * 10;
	}

	for (int i = 0; i < ALPHABET_SIZE; i++) {
        if (game->guessed_letters[i] == 0 && correct) {
            game->guessed_letters[i] = letter;
            break;
        }
    }

	for (int i = 0; i < ALPHABET_SIZE; i++) {
        if (client->guessed_letters[i] == 0) {
            client->guessed_letters[i] = letter;
            break;
        }
    }

	if (send_msg(client->fd, MSG_GUESS_LETTER_OK, nullptr, 0) != 0) {
		perror("send_msg GUESS_LETTER_OK");
    }

	brodcast_game_state(*game);

	// sprawdz czy jest przynajmniej 2 aktywnych graczy
	int active_players = 0;
	for (int i = 0; i < game->player_count; ++i) {
		int pfd = game->players[i];
		auto it = clients.find(pfd);
		if (it == clients.end())
			continue;// Already disconnected
		auto c = it->second;
		if (c->is_active && c->lives > 0) {
			active_players++;
		}
	}
	if (active_players < 2) {
		printf("Gra id=%d zakończona! Za mało aktywnych graczy (%d)\n", game->id, active_players);
		delete_game(game->id);
		return;
	}

	// Sprawdź, czy słowo zostało odgadnięte 
	if(std::strcmp(game->word, game->word_guessed) == 0) {
		printf("tura gry id=%d zakończona! Słowo odgadnięte: %s\n", game->id, game->word);
		broadcast_to_game(*game, MSG_WORD_GUESSED);
		start_new_turn(*game);
		return;
	}
}

void start_new_turn(Game& game) {
	create_game_word(game); // Nowe słowo
	memset(game.guessed_letters, 0, ALPHABET_SIZE);
	memset(game.word_guessed, 0, MAX_WORD_LEN);
	for (int i = 0; i < game.word_length; ++i) {
		game.word_guessed[i] = '_'; // Reset odgadniętych liter
	}
	// Reset stanu graczy
	for (int i = 0; i < game.player_count; ++i) {
		int pfd = game.players[i];
		auto it = clients.find(pfd);
		if (it == clients.end())
			return;// Already disconnected
		auto client = it->second;
		memset(client->guessed_letters, 0, ALPHABET_SIZE);
	}
	printf("Nowa tura gry id=%d rozpoczęta! Nowe słowo: %s\n", game.id, game.word);
	brodcast_game_state(game);
}

void broadcast_to_game(Game& game, uint16_t msg_type, const void* payload, uint16_t length) {
    for (int i = 0; i < game.player_count; ++i) {
        int pfd = game.players[i];
        if (clients.find(pfd) == clients.end())
            continue;

        if (send_msg(pfd, msg_type, payload, length) != 0) {
            perror("send_msg broadcast_to_game");
        }
    }
}

Game* find_game_by_id(uint32_t game_id) {
    auto it = games.find(game_id);
    if (it == games.end())
        return nullptr;
    return &it->second;
}

void handle_start_game(std::shared_ptr<Client> client,  MsgGameIdReq *msg) {
	printf("Gracz %s próbuje rozpocząć grę id=%d\n", client->nick, msg->game_id);
	if (client->state != STATE_IN_ROOM || !client->is_owner) {
		printf("Nieautoryzowana próba rozpoczęcia gry przez %s, state=%d, is_owner=%d\n", client->nick, client->state, client->is_owner);
		if (send_msg(client->fd, MSG_START_GAME_FAIL, nullptr, 0) != 0) {
			perror("send_msg START_GAME_FAIL");
		}
		return;
	}

	Game* game = find_game_by_id(msg->game_id);

	if (game == nullptr || game->active || game->player_count < 2) { // Gra nie istnieje lub już rozpoczęta lub za mało graczy
		printf("Nieprawidłowa próba rozpoczęcia gry id=%d przez %s. Game active=%d, player_count=%d\n", msg->game_id, client->nick, game->active, game->player_count);
		if (send_msg(client->fd, MSG_START_GAME_FAIL, nullptr, 0) != 0) {
			perror("send_msg START_GAME_FAIL");
		}
		return;
	}

	// Start gry
	game->active = 1;
	printf("Gra id=%d rozpoczęta przez %s\n", game->id, client->nick);

	// Inicjalizacja rundy gry

	create_game_word(*game);

	memset(game->guessed_letters, 0, ALPHABET_SIZE);

	for (int i = 0; i < game->word_length; ++i) {
		game->word_guessed[i] = '_'; // Na początku żadna litera nie jest odgadnięta
	}

	// Powiadom graczy o rozpoczęciu gry
	for (int i = 0; i < game->player_count; ++i) {
		int pfd = game->players[i];
		auto it = clients.find(pfd);
		if (it == clients.end())
			return;// Already disconnected
		auto client = it->second;
		client->state = STATE_IN_GAME; 
		client->is_active = 1;
		client->lives = MAX_LIVES;
		client->points = 0;
		memset(client->guessed_letters, 0, ALPHABET_SIZE);
		if (send_msg(pfd, MSG_START_GAME_OK, nullptr, 0) != 0) {
			perror("send_msg START_GAME_OK");
		}
	}
	
	brodcast_game_state(*game);
	broadcast_lobby_state();
}




void create_game_word(Game& game) {
	printf("Tworzenie słowa do gry id=%d\n", game.id);
	memset(game.word, 0, sizeof(game.word));
    static std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> dist(0, WORD_LIST_SIZE - 1);

    std::strncpy(game.word, WORD_LIST[dist(rng)], sizeof(game.word) - 1);
    game.word_length = static_cast<uint8_t>(strlen(game.word));

    for (int i = 0; i < game.word_length; ++i) {
        game.word_guessed[i] = '_';
    }
}

void handle_join_room(std::shared_ptr<Client> client,  MsgGameIdReq *msg) {
	if (client->state != STATE_LOBBY) {
		if (send_msg(client->fd, MSG_JOIN_ROOM_FAIL, nullptr, 0) != 0) {
			perror("send_msg JOIN_ROOM_FAIL");
		}
		return;
	}

	Game* game = find_game_by_id(msg->game_id);

	if (game == nullptr || game->player_count >= MAX_PLAYERS || game->active) {
		if (send_msg(client->fd, MSG_JOIN_ROOM_FAIL, nullptr, 0) != 0) {
			perror("send_msg JOIN_ROOM_FAIL");
		}
		return;
	}

	// Dodaj gracza do gry
	game->players[game->player_count] = client->fd;
	game->player_count++;
	client->game_id = game->id;
	client->is_owner = 0;
	client->lives = MAX_LIVES;
	client->points = 0;
	memset(client->guessed_letters, 0, ALPHABET_SIZE);
	client->state = STATE_IN_ROOM;
	printf("Klient %s dołączył do gry id=%d owner = %s\n", client->nick, game->id, game->owner);
	MsgRoomInfo info;
    memset(&info, 0, sizeof(info));
    info.game_id = game->id;
    info.players_count = game->player_count;
	
    strncpy(info.owner, game->owner, MAX_NICK_LEN - 1);
    info.owner[MAX_NICK_LEN - 1] = '\0';
    // fill player nicknames
    for (uint8_t i = 0; i < info.players_count && i < 8; ++i) {
        int pfd = game->players[i];
        auto cit = clients.find(pfd);
        if (cit != clients.end()) {
            strncpy(info.players[i], cit->second->nick, MAX_NICK_LEN - 1);
            info.players[i][MAX_NICK_LEN - 1] = '\0';
        } else {
            info.players[i][0] = '\0';
        }
    }
	if (send_msg(client->fd, MSG_JOIN_ROOM_OK, &info, sizeof(info)) != 0) {
    perror("send_msg JOIN_ROOM_OK");
	}

	printf("Gry dostępne na serwerze po dołączeniu nowego gracza:\n");

	for (const auto& [id, game] : games) {
		printf("Gry id=%d liczba graczy=%d\n", game.id, game.player_count);
	}

	broadcast_lobby_state();
	brodcast_game_state(*game);
}

void handle_login(std::shared_ptr<Client> client, MsgLoginReq *msg) {
    client->state = STATE_LOGGING_IN;

    char received_nick[MAX_NICK_LEN];
    strncpy(received_nick, msg->nick, MAX_NICK_LEN - 1);
    received_nick[MAX_NICK_LEN - 1] = '\0';

	printf("Logowanie klienta (fd=%d) nick=%s\n", client->fd, received_nick);
    

	if (strcmp(received_nick, "admin") == 0) {
		client->state = STATE_WAIT_ADMIN_PASSWORD;
        strcpy(client->nick, "admin");
        send_msg(client->fd, MSG_ADMIN_PASSWORD_REQUIRED, nullptr, 0);
		printf("Oczekiwanie na hasło admina (fd=%d)\n", client->fd);
        return;
	}

	bool nick_taken = false;

	for (auto const& [fd, other] : clients) {
		if (other == client)
			continue;

		if (other->state >= STATE_LOBBY &&
			strcmp(other->nick, received_nick) == 0) {
			nick_taken = true;
			break;
		}
	}

	if (nick_taken) {
		send_msg(client->fd, MSG_LOGIN_TAKEN, nullptr, 0);
		return;
	}

	strcpy(client->nick, received_nick);
	client->state = STATE_LOBBY;
	send_msg(client->fd, MSG_LOGIN_OK, nullptr, 0);
	send_lobby_state(client);

	printf("Klient zalogowany: %s (fd=%d)\n", client->nick, client->fd);

}

void handle_admin_login(std::shared_ptr<Client> client, MsgAdminLoginReq *msg) {
	if (client->state != STATE_WAIT_ADMIN_PASSWORD) {
		printf("Nieprawidłowa próba logowania admina przez (fd=%d)\n", client->fd);
		send_msg(client->fd, MSG_ADMIN_LOGIN_FAIL, nullptr, 0);
		return;
	}

	if (strcmp(msg->password, admin_pwd) == 0) {
		client->state = STATE_ADMIN;

        send_msg(client->fd, MSG_ADMIN_LOGIN_OK, nullptr, 0);
        printf("Admin zalogowany (fd=%d)\n", client->fd);
    } else {
        send_msg(client->fd, MSG_ADMIN_LOGIN_FAIL, nullptr, 0);
        printf("Nieudana próba logowania admina (fd=%d)\n", client->fd);
    }
}

void handle_admin_list_games(std::shared_ptr<Client> client) {
	if (client->state != STATE_ADMIN) {
		printf("Nieautoryzowana próba pobrania listy gier (fd=%d)\n", client->fd);
		return;
	}
	MsgAdminGamesList list{};
	list.games_count = 0;
	for (const auto& [id, game] : games) {
		if (list.games_count >= MAX_GAMES) break;
		list.games[list.games_count].game_id = game.id;
		list.games[list.games_count].players_count = (uint8_t)game.player_count;
		list.games[list.games_count].is_active = game.active ? 1 : 0;
		strncpy(list.games[list.games_count].owner, game.owner, MAX_NICK_LEN - 1);
		list.games[list.games_count].owner[MAX_NICK_LEN - 1] = '\0';
		list.games_count++;
	}
	send_msg(client->fd, MSG_ADMIN_GAMES_LIST, &list, sizeof(list));
	printf("Admin (fd=%d) pobrał listę %d gier\n", client->fd, list.games_count);
}

void handle_admin_list_users(std::shared_ptr<Client> client) {
	if (client->state != STATE_ADMIN) {
		printf("Nieautoryzowana próba pobrania listy użytkowników (fd=%d)\n", client->fd);
		return;
	}
	
	MsgAdminUsersList list{};
	list.users_count = 0;
	
	for (const auto& [fd, c] : clients) {
		if (list.users_count >= 64) break;
		
		AdminUserInfo& user = list.users[list.users_count];
		strncpy(user.nick, c->nick, MAX_NICK_LEN - 1);
		user.nick[MAX_NICK_LEN - 1] = '\0';
		user.state = (uint8_t)c->state;
		user.game_id = c->game_id;
		
		// Only show lives and points for users who are actually in games (players)
		if (c->game_id != -1 && (c->state == STATE_IN_ROOM || c->state == STATE_IN_GAME)) {
			user.lives = c->lives;
			user.points = c->points;
		} else {
			user.lives = 0;
			user.points = 0;
		}
		
		list.users_count++;
	}
	
	send_msg(client->fd, MSG_ADMIN_USERS_LIST, &list, sizeof(list));
	printf("Admin (fd=%d) pobrał listę %d użytkowników\n", client->fd, list.users_count);
}

void handle_admin_game_details(std::shared_ptr<Client> client, MsgGameIdReq* msg) {
	if (client->state != STATE_ADMIN) {
		printf("Nieautoryzowana próba pobrania szczegółów gry (fd=%d)\n", client->fd);
		return;
	}
	
	Game* game = find_game_by_id(msg->game_id);
	if (game == nullptr) {
		printf("Admin (fd=%d) próbował pobrać szczegóły nieistniejącej gry id=%d\n", client->fd, msg->game_id);
		send_msg(client->fd, MSG_ERROR, nullptr, 0);
		return;
	}
	
	MsgAdminGameDetails details{};
	details.game_id = game->id;
	details.is_active = game->active ? 1 : 0;
	details.player_count = (uint8_t)game->player_count;
	
	strncpy(details.owner, game->owner, MAX_NICK_LEN - 1);
	details.owner[MAX_NICK_LEN - 1] = '\0';
	
	strncpy(details.word, game->word, MAX_WORD_LEN - 1);
	details.word[MAX_WORD_LEN - 1] = '\0';
	
	strncpy(details.word_guessed, game->word_guessed, MAX_WORD_LEN - 1);
	details.word_guessed[MAX_WORD_LEN - 1] = '\0';
	
	memcpy(details.guessed_letters, game->guessed_letters, ALPHABET_SIZE);
	details.word_length = game->word_length;
	
	// Fill player details
	for (int i = 0; i < game->player_count && i < MAX_PLAYERS; ++i) {
		int pfd = game->players[i];
		auto it = clients.find(pfd);
		if (it != clients.end()) {
			auto& c = it->second;
			AdminPlayerInfo& player = details.players[i];
			
			strncpy(player.nick, c->nick, MAX_NICK_LEN - 1);
			player.nick[MAX_NICK_LEN - 1] = '\0';
			player.lives = c->lives;
			player.points = c->points;
			player.is_active = c->is_active ? 1 : 0;
			player.is_owner = c->is_owner ? 1 : 0;
			memcpy(player.guessed_letters, c->guessed_letters, ALPHABET_SIZE);
		}
	}
	
	send_msg(client->fd, MSG_ADMIN_GAME_DETAILS, &details, sizeof(details));
	printf("Admin (fd=%d) pobrał szczegóły gry id=%d\n", client->fd, game->id);
}

void handle_admin_terminate_game(std::shared_ptr<Client> client, MsgGameIdReq* msg) {
	if (client->state != STATE_ADMIN) {
		printf("Nieautoryzowana próba zakończenia gry (fd=%d)\n", client->fd);
		send_msg(client->fd, MSG_ADMIN_TERMINATE_FAIL, nullptr, 0);
		return;
	}
	Game* game = find_game_by_id(msg->game_id);
	if (game == nullptr || !game->active) {
		printf("Nie można zakończyć gry id=%d – nie istnieje lub nieaktywna\n", msg->game_id);
		send_msg(client->fd, MSG_ADMIN_TERMINATE_FAIL, nullptr, 0);
		return;
	}
	delete_game(msg->game_id);
	send_msg(client->fd, MSG_ADMIN_TERMINATE_OK, nullptr, 0);
	// po zakończeniu wyślij adminowi aktualną listę
	handle_admin_list_games(client);
}

void send_game_results(Game& game) {
	// Prepare game results message
	MsgGameResults results{};
	results.game_id = game.id;
	results.player_count = 0;
	
	// Collect all players and their final scores
	for(int i = 0; i < game.player_count; ++i) {
		int pfd = game.players[i];
		auto cit = clients.find(pfd);
		if (cit != clients.end()) {
			auto client = cit->second;
			GameResult& result = results.players[results.player_count];
			strncpy(result.nick, client->nick, MAX_NICK_LEN - 1);
			result.nick[MAX_NICK_LEN - 1] = '\0';
			result.points = client->points;
			result.was_active = client->is_active && client->lives > 0 ? 1 : 0;
			results.player_count++;
		}
	}
	
	// Send results to all players in the game
	for(int i = 0; i < game.player_count; ++i) {
		int pfd = game.players[i];
		if (clients.find(pfd) != clients.end()) {
			printf("  -> Wysyłanie wyników do gracza fd=%d\\n", pfd);
			if (send_msg(pfd, MSG_GAME_RESULTS, &results, sizeof(results)) != 0) {
				perror("send_msg GAME_RESULTS");
			}
		}
	}
	printf("Wysłano wyniki gry id=%d do %d graczy\n", game.id, results.player_count);
}

void delete_game(uint32_t game_id) {
	auto it = games.find(game_id);
	if (it != games.end()) {
		Game& game = it->second;
		
		// Send results BEFORE cleaning up
		send_game_results(game);
		
		// Clean up player states (DON'T send MSG_GAME_END - results are enough)
		game.active = 0;
		for(int i = 0; i < game.player_count; ++i) {
			int pfd = game.players[i];
			auto cit = clients.find(pfd);
			if (cit != clients.end()) {
				auto client = cit->second;
				client->game_id = -1;
				client->state = STATE_LOBBY;
				client->is_owner = 0;
				client->lives = 0;
				client->points = 0;
				memset(client->guessed_letters, 0, ALPHABET_SIZE);
				// MSG_GAME_END not sent here - MSG_GAME_RESULTS is the final message
			}
		}
		games.erase(it);
		printf("Gra id=%d zakończona i usunięta\n", game_id);
		// Wyślij zaktualizowany stan lobby do wszystkich klientów
		broadcast_lobby_state();
	}
	broadcast_lobby_state();
}
void user_exit_room(std::shared_ptr<Client> client) {	
	if (client->game_id == -1 || client->state != STATE_IN_ROOM) {
		printf("Klient %s próbuje opuścić pokj, ale nie jest w zadnym pokoju\n", client->nick);
		if (send_msg(client->fd, MSG_EXIT_ROOM_FAIL, nullptr, 0) != 0) {
			perror("send_msg EXIT_ROOM_FAIL");
		}
		return;
	}
	
	delete_player_from_game(client);

	if (send_msg(client->fd, MSG_EXIT_ROOM_OK, nullptr, 0) != 0) {
		printf("błąd przy wysyłaniu EXIT_ROOM_OK do %s (fd=%d)\n", client->nick, client->fd);
		perror("send_msg EXIT_ROOM_OK");
	}
}

void user_exit_game(std::shared_ptr<Client> client) {
	if (client->game_id == -1) {
		printf("Klient %s próbuje opuścić grę, ale nie jest w żadnej grze\n", client->nick);
		if (send_msg(client->fd, MSG_EXIT_GAME_FAIL, nullptr, 0) != 0) {
			perror("send_msg EXIT_GAME_FAIL");
		}
		return;
	}

	// Save game_id BEFORE delete_player_from_game (which sets it to -1)
	uint32_t saved_game_id = client->game_id;
	
	delete_player_from_game(client);
	
	// MSG_GAME_RESULTS and MSG_GAME_END are sent inside delete_player_from_game if needed
	// Send EXIT_GAME_OK only if game didn't end (still exists)
	Game* game = find_game_by_id(saved_game_id);
	if (game != nullptr) {
		if (send_msg(client->fd, MSG_EXIT_GAME_OK, nullptr, 0) != 0) {
			printf("błąd przy wysyłaniu EXIT_GAME_OK do %s (fd=%d)\n", client->nick, client->fd);
			perror("send_msg EXIT_GAME_OK");
		}
	}
}

void delete_player_from_game(std::shared_ptr<Client> client){
	Game* game = find_game_by_id(client->game_id);
	if (game == nullptr) {
		printf("Klient %s opuszcza grę id=%d\n", client->nick, game->id);
		if (send_msg(client->fd, MSG_EXIT_GAME_FAIL, nullptr, 0) != 0) {
			perror("send_msg EXIT_GAME_FAIL");
		}
		return;
	}
	// delete ROOM when disconeccting player is the owner and the game is not started yet
	// delete GAME with is active (so the game is in progress ) will not occur in this case 
	if (client->is_owner && !game->active) { 
		delete_game(game->id);
		return; // Game deleted, client cleanup done in delete_game
	}
	
	// Check if game will end after removing this player
	bool will_end_game = (game->player_count <= 2 && game->active);

	// IMPORTANT: Send results BEFORE removing player so they're included in the results!
	if (will_end_game) {
		printf("Gra id=%d kończy się - wysyłanie wyników do %d graczy (włącznie z wychodzącym)\n", game->id, game->player_count);
		send_game_results(*game);
	}
	
	// Remove player from game
	auto& players = game->players;
	auto it = std::find(players, players + game->player_count, client->fd);
	if (it != players + game->player_count) {
		std::rotate(it, it + 1, players + game->player_count);
		game->player_count--;
	}

	// Clean up client state
	int exiting_fd = client->fd;
	client->game_id = -1;
	client->state = STATE_LOBBY;
	client->is_owner = 0;
	client->lives = 0;
	client->points = 0;
	memset(client->guessed_letters, 0, ALPHABET_SIZE);

	// Now end the game if needed (MSG_GAME_RESULTS already sent)
	if (will_end_game) {
		// Clean up remaining players (DON'T send MSG_GAME_END - results already sent)
		for(int i = 0; i < game->player_count; ++i) {
			int pfd = game->players[i];
			auto cit = clients.find(pfd);
			if (cit != clients.end()) {
				auto c = cit->second;
				c->game_id = -1;
				c->state = STATE_LOBBY;
				c->is_owner = 0;
				c->lives = 0;
				c->points = 0;
				memset(c->guessed_letters, 0, ALPHABET_SIZE);
				// MSG_GAME_END not sent - MSG_GAME_RESULTS is the final message
			}
		}
		games.erase(game->id);
		printf("Gra id=%d zakończona z powodu braku graczy\n", game->id);
		broadcast_lobby_state();
	} else {
		// Jeśli gra nadal istnieje, ale gracz opuścił, też zaktualizuj lobby
		broadcast_lobby_state();
		brodcast_game_state(*game);
	}
}

void disconnect_client(int cfd, int epoll_fd) {
	auto it = clients.find(cfd);
    if (it == clients.end())
        return;// Already disconnected

    auto client = it->second;

	// Usuń klienta z gry, jeśli jest w jakiejś
	if( client->game_id != -1) {
		delete_player_from_game(client);
	}

    if (epoll_fd > 0) {
        epoll_ctl(epoll_fd, EPOLL_CTL_DEL, cfd, nullptr);
    }
    
    close(cfd);
    clients.erase(cfd);

    printf("Klient usunięty (fd=%d)\n", cfd);
}

void handle_create_room(std::shared_ptr<Client> client) {
	printf("Obsługa CREATE_ROOM_REQ od %s (fd=%d)\n", client->nick, client->fd);
    if (client->state != STATE_LOBBY) {
		if (send_msg(client->fd, MSG_CREATE_ROOM_FAIL, nullptr, 0) != 0) {
			perror("send_msg CREATE_GAME_FAIL");
		}
        return;
    }

    Game game;
    memset(&game, 0, sizeof(Game));

    // ID gry
	game.id = next_game_id++;
	
    // Owner
    strncpy(game.owner, client->nick, sizeof(client->nick) - 1);
	game.owner[sizeof(game.owner) - 1] = '\0';

    // Gracze
    game.players[0] = client->fd;
    game.player_count = 1;

    // Stan gry
    game.active = 0;


    // Słowo (na razie puste)
    memset(game.word, 0, sizeof(game.word));


    games.emplace(game.id, game);

    client->game_id = game.id;
	client->is_owner = 1;
	client->lives = MAX_LIVES;
	client->points = 0;
	memset(client->guessed_letters, 0, ALPHABET_SIZE);
	client->state = STATE_IN_ROOM;

    printf("Utworzono grę id=%d owner=%s\n",game.id, game.owner);


    // Powiadom twórcę 
	printf("Wysyłam CREATE_ROOM_OK do %s (fd=%d)\n", client->nick, client->fd);
    if (send_msg(client->fd, MSG_CREATE_ROOM_OK, nullptr, 0) != 0) {
		perror("send_msg CREATE_ROOM_OK");
	}

	// prześlij stan aktulany nowo stworzonej gry
	MsgRoomInfo info;
    memset(&info, 0, sizeof(info));
    info.game_id = game.id;
    info.players_count = game.player_count;
    strncpy(info.owner, game.owner, MAX_NICK_LEN - 1);
    info.owner[MAX_NICK_LEN - 1] = '\0';
    // fill player nicknames
    for (uint8_t i = 0; i < info.players_count && i < 8; ++i) {
        int pfd = game.players[i];
        auto cit = clients.find(pfd);
        if (cit != clients.end()) {
            strncpy(info.players[i], cit->second->nick, MAX_NICK_LEN - 1);
            info.players[i][MAX_NICK_LEN - 1] = '\0';
        } else {
            info.players[i][0] = '\0';
        }
    }
    if (send_msg(client->fd, MSG_JOIN_ROOM_OK, &info, sizeof(info)	) != 0) {
		perror("send_msg JOIN_ROOM_OK");
	}

	
	printf("Gry dostępne na serwerze:\n");

	for (const auto& [id, game] : games) {
		printf("Gry id=%d liczba graczy=%d\n", game.id, game.player_count);
	}

	broadcast_lobby_state();
}

void send_lobby_state(std::shared_ptr<Client> client) {
    MsgLobbyState msg;
    memset(&msg, 0, sizeof(msg));

    msg.games_count = 0;

	for (const auto& [id, game] : games) {
		if (msg.games_count >= MAX_GAMES)
			break;
		if (game.active)
			continue;
		msg.games[msg.games_count].game_id = game.id;
		msg.games[msg.games_count].players_count = game.player_count;
		msg.games_count++;
	}

    send_msg(client->fd, MSG_LOBBY_STATE, &msg, sizeof(msg));
}

void broadcast_lobby_state() { // TODO tylko jak gracz jest w lobby
    for (auto& [fd, other] : clients) {
        if (other->state == STATE_LOBBY) {
            send_lobby_state(other);
        }
    }
}

void brodcast_game_state(Game& game) {
    MsgGameState game_state;
    memset(&game_state, 0, sizeof(MsgGameState));
    
    game_state.game_id = game.id;
    game_state.word_length = game.word_length;
    game_state.player_count = game.player_count;

    memcpy(game_state.word, game.word_guessed, MAX_WORD_LEN);
    memcpy(game_state.guessed_letters, game.guessed_letters, ALPHABET_SIZE);

    printf("Wysyłam game_state.word: %s, dla %s\n", game_state.word, game.word);
    printf("Wysyłam game_state.guessed_letters: %s\n", game_state.guessed_letters);

    // Gracze
    for (int i = 0; i < game.player_count; ++i) {
        int pfd = game.players[i];
        auto cit = clients.find(pfd);
        if (cit != clients.end()) {
            auto& c = cit->second;
            NetPlayerState& ps = game_state.players[i];
            strncpy(ps.nick, c->nick, MAX_NICK_LEN - 1);
            ps.lives = c->lives;
            ps.points = c->points;
            memcpy(ps.guessed_letters, c->guessed_letters, ALPHABET_SIZE);
            ps.is_owner = c->is_owner;
            ps.is_active = c->is_active;
        }
    }

	broadcast_to_game(game, MSG_GAME_STATE, &game_state, sizeof(game_state));
}
void handle_shutdown(int sig) {
    server_running = 0;
}

void graceful_shutdown() {
    printf("Serwer się wyłącza – informuję klientów\n");

    for (auto& [fd, client] : clients) {
        if(client->state == STATE_IN_GAME){
            user_exit_game(client);
        }
        if(client->state == STATE_IN_ROOM){
            user_exit_room(client);
        }
        send_msg(fd, MSG_SERVER_SHUTDOWN, nullptr, 0);
        shutdown(fd, SHUT_RDWR); // wysyła FIN
        close(fd);
    }

    clients.clear();
}