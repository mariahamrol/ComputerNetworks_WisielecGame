#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

#include "../common/protocol.h"
#include "../common/messages.h"
#include "../common/states.h"
#include "../common/net.hpp"

// Adres i port serwera
#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 12345

bool handle_login(int sfd);
void client_event_loop(int sfd);
void handle_lobby_message(int sfd, MsgHeader& hdr, char* buffer);

void print_games_list(MsgLobbyState* msg);
void handle_lobby(int sfd);

int main() {
    int sfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sfd < 0) {
        perror("socket");
        return 1;
    }

    sockaddr_in sa{};
    sa.sin_family = AF_INET;
    sa.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_IP, &sa.sin_addr);

    if (connect(sfd, (sockaddr*)&sa, sizeof(sa)) < 0) {
        perror("connect");
        close(sfd);
        return 1;
    }

    std::cout << "Połączono z serwerem.\n";

    if (!handle_login(sfd)) {
        close(sfd);
        return 0;
    }

    client_event_loop(sfd);

    close(sfd);
    std::cout << "Rozłączono.\n";
    return 0;
}

void client_event_loop(int sfd) {
    while (true) {
        MsgHeader hdr;
        char buffer[4096];

        int r = recv_msg(sfd, hdr, buffer, sizeof(buffer));
        if (r <= 0) {
            std::cerr << "Utracono połączenie z serwerem.\n";
            return;
        }

        handle_lobby_message(sfd, hdr, buffer);
    }
}

void handle_lobby_message(int sfd, MsgHeader& hdr, char* buffer) {
    switch (hdr.type) {

        case MSG_LOBBY_STATE: {
            auto* lobby = (MsgLobbyState*)buffer;
            print_games_list(lobby);

            std::cout << "ID | c = create | q = quit\n> ";
            std::string choice;
            std::cin >> choice;

            if (choice == "q") {
                exit(0);
            }
            else if (choice == "c") {
                send_msg(sfd, MSG_CREATE_GAME_REQ, nullptr, 0);
            }
            else {
                try {
                    MsgJoinGameReq req;
                    req.game_id = std::stoul(choice);
                    send_msg(sfd, MSG_JOIN_GAME_REQ, &req, sizeof(req));
                } catch (...) {
                    std::cerr << "Nieprawidłowy wybór.\n";
                }
            }
            break;
        }

        case MSG_JOIN_GAME_OK:
            std::cout << "Dołączono do gry.\n";
            // tutaj w przyszłości: game_loop()
            break;

        case MSG_JOIN_GAME_FAIL:
            std::cerr << "Nie udało się dołączyć do gry.\n";
            break;

        default:
            std::cerr << "Nieznany typ wiadomości: " << hdr.type << "\n";
    }
}


bool handle_login(int sfd) {
    std::string nick;
    std::cout << "Podaj swój nick: ";
    std::cin >> nick;

    MsgLoginReq req{};
    strncpy(req.nick, nick.c_str(), MAX_NICK_LEN - 1);

    if (send_msg(sfd, MSG_LOGIN_REQ, &req, sizeof(req)) != 0) {
        std::cerr << "Błąd wysyłania loginu.\n";
        return false;
    }

    MsgHeader hdr;
    char buffer[4096];

    int r = recv_msg(sfd, hdr, buffer, sizeof(buffer));
    if (r <= 0) {
        std::cerr << "Serwer zamknął połączenie.\n";
        return false;
    }

    if (hdr.type == MSG_LOGIN_OK) {
        std::cout << "Zalogowano jako: " << nick << "\n";
        return true;
    }

    if (hdr.type == MSG_LOGIN_TAKEN) {
        std::cerr << "Nick zajęty.\n";
        return false;
    }

    std::cerr << "Nieoczekiwana odpowiedź: " << hdr.type << "\n";
    return false;
}



void handle_lobby(int sfd) {
    // Po zalogowaniu serwer powinien automatycznie przysłać stan lobby (MSG_LOBBY_STATE)
    // Czekamy na tę wiadomość w pętli.
    while (true) {
        MsgHeader hdr;
        char buffer[4096];
        int r = recv_msg(sfd, hdr, buffer, sizeof(buffer));

        if (r <= 0) {
            std::cerr << "Serwer zamknął połączenie lub wystąpił błąd." << std::endl;
            return;
        }

        switch(hdr.type) {
            case MSG_LOBBY_STATE: {
                MsgLobbyState* lobby_state = (MsgLobbyState*)buffer;
                print_games_list(lobby_state);

                std::cout << std::endl;
                std::cout << "Wpisz ID pokoju, aby dołączyć." << std::endl;
                std::cout << "Wpisz 'c', aby stworzyć nowy pokój." << std::endl;
                std::cout << "Wpisz 'q', aby wyjść." << std::endl;
                std::cout << "> ";

                std::string choice;
                std::cin >> choice;

                if (choice == "q") {
                    return; // Wyjście z handle_lobby i zamknięcie klienta
                } else if (choice == "c") {
                    // Po implementacji na serwerze, wyślij MSG_CREATE_GAME_REQ
                    if (send_msg(sfd, MSG_CREATE_GAME_REQ, NULL, 0) != 0) {
                         std::cerr << "Nie udało się wysłać prośby o stworzenie gry." << std::endl;
                    }
					
                    // Po wysłaniu prośby serwer powinien odesłać nowy MSG_LOBBY_STATE
                } else {
                    try {
                        uint32_t game_id = std::stoul(choice);
                        MsgJoinGameReq join_req;
                        join_req.game_id = game_id;
                        if (send_msg(sfd, MSG_JOIN_GAME_REQ, &join_req, sizeof(join_req)) != 0) {
                            std::cerr << "Nie udało się wysłać prośby o dołączenie do gry." << std::endl;
                        }
                    } catch (const std::exception& e) {
                        std::cerr << "Nieprawidłowy wybór. Oczekiwanie na nowy stan lobby..." << std::endl;
                    }
                }
                break;
            }
            case MSG_JOIN_GAME_OK: {
                std::cout << "Dołączono do gry! Oczekiwanie na drugiego gracza..." << std::endl;
                // TODO: Dalsza logika gry
                // W tym miejscu powinna rozpocząć się pętla obsługująca stan w grze,
                // czekająca na wiadomości od serwera. Na razie kończymy.
                return;
            }
            case MSG_JOIN_GAME_FAIL: {
                std::cerr << "Nie udało się dołączyć do gry (może być pełna lub nie istnieje)." << std::endl;
                std::cerr << "Oczekiwanie na nowy stan lobby..." << std::endl;
                break; // Czekaj na kolejny MSG_LOBBY_STATE
            }
            default:
                std::cerr << "Otrzymano nieoczekiwaną wiadomość w lobby: " << hdr.type << std::endl;
        }
    }
}

void print_games_list(MsgLobbyState* msg) {
    std::cout << "\n--- Dostępne pokoje ---" << std::endl;
    if (msg->games_count == 0) {
        std::cout << "Brak dostępnych pokoi." << std::endl;
    } else {
        for (uint32_t i = 0; i < msg->games_count; ++i) {
            std::cout << "  * Pokój " << msg->games[i].game_id 
                      << " (" << (int)msg->games[i].players_count << "/8 graczy)" << std::endl;
        }
    }
    std::cout << "------------------------" << std::endl;
}
