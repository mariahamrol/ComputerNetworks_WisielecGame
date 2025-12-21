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
	client.onGameState = [](const MsgGameState& msg) {
        std::cout << "=== Game State ===\n";
		std::cout << "Gra ID: " << msg.game_id << "\n";
		std::cout << "Słowo: ";
		for (int i = 0; i < msg.word_length; ++i) {
			if (msg.word[i]) {
				std::cout << msg.word[i] << " ";
			} else {
				std::cout << "* ";
			}
		}
		for (int i = 0; i < msg.player_count; ++i) {
			const NetPlayerState& p = msg.players[i];
			std::cout << "\nGracz: " << p.nick
					  << " Lives: " << (int)p.lives
					  << " Points: " << p.points
					  << " Active: " << (p.is_active ? "Yes" : "No")
					  << " Owner: " << (p.is_owner ? "Yes" : "No")
					  << " Guessed letters: ";
			for (int j = 0; j < ALPHABET_SIZE; ++j) {
				if (p.guessed_letters[j]) {
					std::cout << p.guessed_letters[j] << " ";
				}
			}
		}
		

        std::cout << "\n";
    };

    client.onCreateRoomOk = [] {
        std::cout << "Utworzono pokój\n";
    };

	client.onJoinRoomOk = [] {
		std::cout << "Dołączono do pokoju\n";
	};
	client.onJoinRoomFail = [] {
		std::cout << "Nie udało się dołączyć do pokoju\n";
	};
	client.onStartGameOk = [] {
		std::cout << "Gra rozpoczęta\n";
	};
	client.onStartGameFail = [] {
		std::cout << "Nie udało się rozpocząć gry\n";
	};
	client.onGuessLetterOk = [] {
		std::cout << "Litera przeanalizowana poprawnie\n";
	};
	client.onGuessLetterFail = [] {
		std::cout << "Litera już odgadnięta lub nieprawidłowa\n";
	};
	client.onGameEnd = [] {
		std::cout << "Gra zakończona\n";
	};
	client.onPlayerEliminated = [] {
		std::cout << "Zostałeś wyeliminowany z gry!!!\n";
	};
	client.onWordGuessed = [] {
		std::cout << "Gratulacje! Słowo zostało odgadnięte!\n";
		std::cout << "Odgadujemy nowe słowo!\n";
	};
	client.onExitGameOk = [] {
		std::cout << "Pomyślnie opuściłeś grę\n";
	};
	client.onExitGameFail = [] {
		std::cout << "Nie można opuścić gry\n";
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
		 std::cout << "c - crate game, q - quit, l - list games, j - join game, s - start game, g - guess letter, e - exit game: ";
        std::cin >> c;

        if (c == 'c') {
            client.createRoom();
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
			uint32_t roomId;
			std::cout << "ID pokoju do dołączenia: ";
			std::cin >> roomId;
			client.joinRoom(roomId);
		}
		else if (c == 's') {
			uint32_t roomId;
			std::cout << "ID pokoju do rozpoczęcia: ";
			std::cin >> roomId;
			client.startGame(roomId);	
		}
		else if (c == 'g') {
			char letter;
			std::cout << "Litera do odgadnięcia: ";
			std::cin >> letter;
			client.guessLetter(letter);
		}
		else if (c == 'e') {
			std::cout << "Opuszczanie gry...\n";
			client.exitGame();
		}
	}

    client.disconnect();
}
