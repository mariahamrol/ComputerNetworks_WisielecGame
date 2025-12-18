#include <iostream>
#include "../client/ClientConnection.h"

int main() {
    ClientConnection client;

    // --- reakcje na zdarzenia ---
    client.onLoginOk = [] {
        std::cout << "Zalogowano OK\n";
    };

    client.onLoginTaken = [] {
        std::cout << "Nick zajęty\n";
    };

    client.onLobbyState = [](const MsgLobbyState& msg) {
        std::cout << "=== Lobby ===\n";
        for (uint32_t i = 0; i < msg.games_count; ++i) {
            std::cout << "Gra "
                      << msg.games[i].game_id
                      << " ("
                      << (int)msg.games[i].players_count
                      << " graczy)\n";
        }
    };

    client.onCreateGameOk = [] {
        std::cout << "Utworzono grę\n";
    };

    // --- start ---
    if (!client.connectToServer("127.0.0.1", 12345)) {
        std::cerr << "Nie można połączyć\n";
        return 1;
    }

    std::string nick;
    std::cout << "Nick: ";
    std::cin >> nick;

    client.login(nick);



    // --- pętla UI (TYLKO UI!) ---
    while (true) {
        char c;
		 std::cout << "c - crate game, q - quit, l - list games, j - join game: ";
        std::cin >> c;

        if (c == 'c') {
            client.createGame();
        }
        if (c == 'q') {
            break;
        }
		else if (c == 'l') {
			auto lobby = client.getLastLobbyState();
			if (!lobby) {
				std::cout << "Brak danych lobby (jeszcze nie otrzymano)\n";
			} else {
				std::cout << "\n=== Lobby ===\n";
				for (uint32_t i = 0; i < lobby->games_count; ++i) {
					std::cout << "Gra "
							<< lobby->games[i].game_id
							<< " ("
							<< (int)lobby->games[i].players_count
							<< " graczy)\n";
				}
				std::cout << "=============\n";
			}
		}
		else if (c == 'j') {
			uint32_t gameId;
			std::cout << "ID gry do dołączenia: ";
			std::cin >> gameId;
			client.joinGame(gameId);
		}
    }

    client.disconnect();
}
