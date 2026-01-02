#include <iostream>
#include "../client/ClientConnection.h"

int main() {
	ClientConnection client;
	bool adminMode = false;

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

    client.onCreateRoomOk = [](uint32_t gameId, uint8_t playersCount, const std::string &owner, const std::vector<std::string> &players) {
        std::cout << "Utworzono pokój id=" << gameId << " owner=" << owner << " players=" << (int)playersCount << "\n";
    };

	client.onJoinRoomOk = [](uint32_t gameId, uint8_t playersCount, const std::string &owner, const std::vector<std::string> &players) {
		std::cout << "Dołączono do pokoju id=" << gameId << " owner=" << owner << " players=" << (int)playersCount << "\n";
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
		std::cout << "-------------------------------\n";
		std::cout << "Zostałeś wyeliminowany z gry!!!\n";
		std::cout << "-------------------------------\n";
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
	client.onExitRoomOk = [] {
		std::cout << "Pomyślnie opuściłeś pokj\n";
	};
	client.onExitRoomFail = [] {
		std::cout << "Nie można opuścić pokoju\n";
	};
	client.onError = [](const std::string& msg) {
		std::cout << msg << "\n";
	};
	client.onAdminGamesList = [](const MsgAdminGamesList& list) {
		std::cout << "\n=== Wszystkie gry (admin) ===\n";
		uint32_t totalPlayers = 0;
		for (uint32_t i = 0; i < list.games_count; ++i) {
			std::cout << "Gra " << list.games[i].game_id
					  << " | Graczy: " << (int)list.games[i].players_count
					  << " | Status: " << (list.games[i].is_active ? "AKTYWNA" : "POKÓJ")
					  << " | Owner: " << list.games[i].owner << "\n";
			totalPlayers += list.games[i].players_count;
		}
		std::cout << "Razem gier: " << list.games_count << ", razem graczy: " << totalPlayers << "\n";
	};
	client.onAdminTerminateOk = [] {
		std::cout << "Gra zakończona przez admina\n";
	};
	client.onAdminTerminateFail = [] {
		std::cout << "Nie udało się zakończyć gry (admin)\n";
	};
	client.onAdminPasswordRequired = [] {
		std::cout << "Wymagane hasło admina\n";
	};
	client.onAdminLoginOk = [&adminMode] {
		adminMode = true;
		std::cout << "Zalogowano jako admin\n";
	};
	client.onAdminLoginFail = [] {
		std::cout << "Błędne hasło admina\n";
	};
	client.onAdminUsersList = [](const MsgAdminUsersList& list) {
		std::cout << "\n=== Wszyscy użytkownicy (admin) ===\n";
		const char* states[] = {"CONNECTED", "LOGGING_IN", "WAIT_ADMIN_PWD", "LOBBY", "IN_ROOM", "IN_GAME", "ADMIN"};
		for (uint32_t i = 0; i < list.users_count; ++i) {
			const auto& user = list.users[i];
			std::cout << "Nick: " << user.nick
					  << " | Stan: " << (user.state < 7 ? states[user.state] : "UNKNOWN")
					  << " | Gra: " << (user.game_id == -1 ? "brak" : std::to_string(user.game_id))
					  << " | Życia: " << (int)user.lives
					  << " | Punkty: " << user.points << "\n";
		}
		std::cout << "Razem użytkowników: " << list.users_count << "\n";
	};
	client.onAdminGameDetails = [](const MsgAdminGameDetails& details) {
		std::cout << "\n=== Szczegóły gry ID: " << details.game_id << " (admin) ===\n";
		std::cout << "Status: " << (details.is_active ? "AKTYWNA" : "POKÓJ") << "\n";
		std::cout << "Owner: " << details.owner << "\n";
		std::cout << "Graczy: " << (int)details.player_count << "\n";
		if (details.is_active) {
			std::cout << "\n*** SŁOWO (admin): " << details.word << " ***\n";
			std::cout << "Stan odgadywania: ";
			for (int i = 0; i < details.word_length; ++i) {
				std::cout << (details.word_guessed[i] ? details.word_guessed[i] : '_') << " ";
			}
			std::cout << "\nOdgadnięte litery (wszyscy): ";
			for (int i = 0; i < ALPHABET_SIZE && details.guessed_letters[i]; ++i) {
				std::cout << details.guessed_letters[i] << " ";
			}
			std::cout << "\n";
		}
		std::cout << "\n--- Gracze ---\n";
		for (int i = 0; i < details.player_count; ++i) {
			const auto& p = details.players[i];
			std::cout << (i + 1) << ". " << p.nick
					  << " | Życia: " << (int)p.lives
					  << " | Punkty: " << p.points
					  << " | Aktywny: " << (p.is_active ? "TAK" : "NIE")
					  << " | Owner: " << (p.is_owner ? "TAK" : "NIE");
			if (details.is_active) {
				std::cout << "\n   Odgadnięte przez tego gracza: ";
				for (int j = 0; j < ALPHABET_SIZE && p.guessed_letters[j]; ++j) {
					std::cout << p.guessed_letters[j] << " ";
				}
			}
			std::cout << "\n";
		}
		std::cout << "==================\n";
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
	if (nick == "admin") {
		std::string pwd;
		std::cout << "Hasło admina: ";
		std::cin >> pwd;
		client.adminLogin(pwd);
	}



    // --- pętla UI (TYLKO UI!) ---
	while (true) {
		char c;
		if (adminMode) {
			std::cout << "\n[ADMIN] a - lista gier | u - użytkownicy | i - info o grze | x - zakończ grę | q - wyjdź: ";
		} else {
			std::cout << "\nc - create game | l - list games | j - join | s - start | g - guess | e - exit game | d - exit room | q - quit: ";
		}
		std::cin >> c;

		if (!adminMode && c == 'c') {
            client.createRoom();
		}
        if (c == 'q') {
            break;
        }
		else if (adminMode && c == 'a') {
			client.adminListGames();
		}
		else if (adminMode && c == 'u') {
			client.adminListUsers();
		}
		else if (adminMode && c == 'i') {
			uint32_t gid;
			std::cout << "ID gry do sprawdzenia: ";
			std::cin >> gid;
			client.adminGetGameDetails(gid);
		}
		else if (adminMode && c == 'x') {
			uint32_t gid;
			std::cout << "ID gry do zakończenia: ";
			std::cin >> gid;
			client.adminTerminateGame(gid);
		}
		else if (!adminMode && c == 'l') {
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
		else if (!adminMode && c == 'j') {
			uint32_t roomId;
			std::cout << "ID pokoju do dołączenia: ";
			std::cin >> roomId;
			client.joinRoom(roomId);
		}
		else if (!adminMode && c == 's') {
			uint32_t roomId;
			std::cout << "ID pokoju do rozpoczęcia: ";
			std::cin >> roomId;
			client.startGame(roomId);	
		}
		else if (!adminMode && c == 'g') {
			char letter;
			std::cout << "Litera do odgadnięcia: ";
			std::cin >> letter;
			client.guessLetter(letter);
		}
		else if (!adminMode && c == 'e') {
			std::cout << "Opuszczanie gry...\n";
			client.exitGame();
		}
		else if (!adminMode && c == 'd') {
			std::cout << "Opuszczanie pokoju...\n";
			client.exitRoom();
		}
	}

    client.disconnect();
}
