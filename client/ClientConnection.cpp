#include "ClientConnection.h"
#include "../include/net.hpp"

#include <unistd.h>
#include <arpa/inet.h>
#include <cstring>
#include <vector>

ClientConnection::ClientConnection() {}

ClientConnection::~ClientConnection() {
    disconnect();
}

std::optional<MsgLobbyState> ClientConnection::getLastLobbyState() {
    std::lock_guard<std::mutex> lock(lobbyMutex);
    return lastLobbyState;
}
std::optional<MsgGameState> ClientConnection::getLastGameState() {
    std::lock_guard<std::mutex> lock(gameStateMutex);
    return lastGameState;
}


bool ClientConnection::connectToServer(const std::string& ip, int port) {
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) return false;

    sockaddr_in sa{};
    sa.sin_family = AF_INET;
    sa.sin_port = htons(port);
    inet_pton(AF_INET, ip.c_str(), &sa.sin_addr);

    if (connect(sock, (sockaddr*)&sa, sizeof(sa)) < 0) {
        close(sock);
        sock = -1;
        return false;
    }

    running = true;
    recvThread = std::thread(&ClientConnection::recvLoop, this);
    return true;
}

void ClientConnection::disconnect() {
    running = false;
    if (sock >= 0) {
        close(sock);
        sock = -1;
    }
    if (recvThread.joinable())
        recvThread.join();
}

void ClientConnection::login(const std::string& nick) {
    MsgLoginReq req{};
    strncpy(req.nick, nick.c_str(), MAX_NICK_LEN - 1);
    send_msg(sock, MSG_LOGIN_REQ, &req, sizeof(req));
}

void ClientConnection::createRoom() {
    send_msg(sock, MSG_CREATE_ROOM_REQ, nullptr, 0);
}

void ClientConnection::joinRoom(uint32_t id) {
    MsgGameIdReq req{ id };
    send_msg(sock, MSG_JOIN_ROOM_REQ, &req, sizeof(req));
}
void ClientConnection::startGame(uint32_t id) {
	MsgGameIdReq req{ id };
	send_msg(sock, MSG_START_GAME_REQ, &req, sizeof(req));
}
void ClientConnection::guessLetter(char letter) {
	MsgGuessLetterReq req{ letter };
	send_msg(sock, MSG_GUESS_LETTER_REQ, &req, sizeof(req));
}
void ClientConnection::exitGame() {
	send_msg(sock, MSG_EXIT_GAME_REQ, nullptr, 0);
}
void ClientConnection::exitRoom() {
	send_msg(sock, MSG_EXIT_ROOM_REQ, nullptr, 0);
}

void ClientConnection::recvLoop() {
    char buffer[4096];

    while (running) {
        MsgHeader hdr;
        int r = recv_msg(sock, hdr, buffer, sizeof(buffer));
        if (r <= 0) {
            if (onError) onError("Disconnected from server");
            running = false;
            break;
        }
        handleMessage(hdr, buffer);
    }
}

void ClientConnection::handleMessage(MsgHeader& hdr, char* payload) {
    switch (hdr.type) {
		case MSG_GAME_STATE: {
			MsgGameState copy = *(MsgGameState*)payload;
			{
				std::lock_guard<std::mutex> lock(gameStateMutex);
				lastGameState = copy;
			}
			if (onGameState)
				onGameState(copy);
			break;
		}
        case MSG_LOGIN_OK:
            if (onLoginOk) onLoginOk();
            break;
        case MSG_LOGIN_TAKEN:
            if (onLoginTaken) onLoginTaken();
            break;
		case MSG_LOBBY_STATE: {
			MsgLobbyState copy = *(MsgLobbyState*)payload;
			{
				std::lock_guard<std::mutex> lock(lobbyMutex);
				lastLobbyState = copy;
			}
			if (onLobbyState)
				onLobbyState(copy);
			break;
		}
        case MSG_CREATE_ROOM_OK: {
            MsgRoomInfo info = *(MsgRoomInfo*)payload;
            std::vector<std::string> players;
            for (uint8_t i = 0; i < info.players_count && i < 8; ++i) {
                players.emplace_back(info.players[i]);
            }
            if (onCreateRoomOk)
                onCreateRoomOk(info.game_id, info.players_count, std::string(info.owner), players);
            break;
        }
        case MSG_JOIN_ROOM_OK: {
            MsgRoomInfo info = *(MsgRoomInfo*)payload;
            std::vector<std::string> players;
            for (uint8_t i = 0; i < info.players_count && i < 8; ++i) {
                players.emplace_back(info.players[i]);
            }
            if (onJoinRoomOk)
                onJoinRoomOk(info.game_id, info.players_count, std::string(info.owner), players);
            break;
        }
		case MSG_JOIN_ROOM_FAIL:
			if (onJoinRoomFail) onJoinRoomFail();
			break;
		case MSG_START_GAME_OK:
			if (onStartGameOk) onStartGameOk();
			break;
        // case MSG_START_GAME_OK: {
        //     auto* msg = (MsgStartGameOk*)payload;

        //     if (onStartGameOk)
        //         onStartGameOk(
        //             msg->game_id,
        //             std::string(msg->hidden_word)
        //         );
        //     break;
        // }

		case MSG_START_GAME_FAIL:
			if (onStartGameFail) onStartGameFail();
			break;
		case MSG_GUESS_LETTER_OK:
			if (onGuessLetterOk) onGuessLetterOk();
			break;
		case MSG_GUESS_LETTER_FAIL:
			if (onGuessLetterFail) onGuessLetterFail();
			break;
		case MSG_GAME_END:
			if (onGameEnd) onGameEnd();
			break;
		case MSG_PLAYER_ELIMINATED:
			if (onPlayerEliminated) onPlayerEliminated();
			break;
		case MSG_WORD_GUESSED:
			if (onWordGuessed) onWordGuessed();
			break;
		case MSG_EXIT_GAME_OK:
			if (onExitGameOk) onExitGameOk();
			break;
		case MSG_EXIT_GAME_FAIL:
			if (onExitGameFail) onExitGameFail();
			break;
		case MSG_SERVER_SHUTDOWN:
			if (onError) onError("Server is shutting down");
			running = false;
			break;
		case MSG_EXIT_ROOM_OK:
			if (onExitRoomOk) onExitRoomOk();
			break;
		case MSG_EXIT_ROOM_FAIL:
			if (onExitRoomFail) onExitRoomFail();
			break;
        default:
			// Nieznany typ wiadomości
			break;
    }
}
