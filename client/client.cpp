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

void print_games_list(MsgLobbyState* msg);
void handle_lobby(int sfd);

int main(int argc, char** argv) {
    int sfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sfd < 0) {
        perror("socket");
        return 1;
    }

    struct sockaddr_in sa;
    memset(&sa, 0, sizeof(sa));
    sa.sin_family = AF_INET;
    sa.sin_port = htons(SERVER_PORT);
    if (inet_pton(AF_INET, SERVER_IP, &sa.sin_addr) <= 0) {
        perror("inet_pton");
        close(sfd);
        return 1;
    }

    if (connect(sfd, (struct sockaddr*)&sa, sizeof(sa)) < 0) {
        perror("connect");
        close(sfd);
        return 1;
    }

    std::cout << "Połączono z serwerem." << std::endl;

    // --- Logowanie ---
    std::string nick;
    std::cout << "Podaj swój nick: ";
    std::cin >> nick;

    MsgLoginReq login_req;
    strncpy(login_req.nick, nick.c_str(), MAX_NICK_LEN - 1);
    login_req.nick[MAX_NICK_LEN - 1] = '\0';

    if (send_msg(sfd, MSG_LOGIN_REQ, &login_req, sizeof(login_req)) != 0) {
        std::cerr << "Nie udało się wysłać prośby o logowanie." << std::endl;
        close(sfd);
        return 1;
    }

    // --- Oczekiwanie na odpowiedź serwera ---
    MsgHeader hdr;
    char buffer[4096];
    int r = recv_msg(sfd, hdr, buffer, sizeof(buffer));
    if (r <= 0) {
        std::cerr << "Serwer zamknął połączenie lub wystąpił błąd." << std::endl;
        close(sfd);
        return 1;
    }

    if (hdr.type == MSG_LOGIN_OK) {
        std::cout << "Zalogowano pomyślnie jako: " << nick << std::endl;
        std::cout << "Wchodzę do lobby..." << std::endl;
        handle_lobby(sfd);
    } else if (hdr.type == MSG_LOGIN_TAKEN) {
        std::cerr << "Nick jest już zajęty." << std::endl;
    } else {
        std::cerr << "Otrzymano nieoczekiwaną odpowiedź od serwera: " << hdr.type << std::endl;
    }

    std::cout << "Rozłączono." << std::endl;
    close(sfd);
    return 0;
}

// Funkcja obsługująca interakcje w lobby (wybór gry)
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
                      << " (" << (int)msg->games[i].players_count << "/2 graczy)" << std::endl;
        }
    }
    std::cout << "------------------------" << std::endl;
}
