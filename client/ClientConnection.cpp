#include "ClientConnection.h"
#include "../include/net.hpp"

#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <cstring>
#include <vector>
#include <iostream>

ClientConnection::ClientConnection() {}

ClientConnection::~ClientConnection() {
    disconnect();
}

std::optional<MsgLobbyState> ClientConnection::getLastLobbyState() {
    std::lock_guard<std::mutex> lock(lobbyMutex);
    return lastLobbyState;
}

bool ClientConnection::connectToServer(const std::string& ip, int port) {
	disconnect();

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) return false;

    sockaddr_in sa{};
    sa.sin_family = AF_INET;
    sa.sin_port = htons(port);
    
    // Validate IP address format
    if (inet_pton(AF_INET, ip.c_str(), &sa.sin_addr) <= 0) {
        std::cerr << "Invalid IP address format: " << ip << std::endl;
        close(sock);
        sock = -1;
        return false;
    }

    // Set socket to non-blocking mode for connect timeout
    int flags = fcntl(sock, F_GETFL, 0);
    fcntl(sock, F_SETFL, flags | O_NONBLOCK);

    // Try to connect (will return immediately with EINPROGRESS)
    int result = connect(sock, reinterpret_cast<sockaddr*>(&sa), sizeof(sa));
    
    if (result < 0) {
        if (errno != EINPROGRESS) {
            std::cerr << "Failed to connect to " << ip << ":" << port << " - " << strerror(errno) << std::endl;
            close(sock);
            sock = -1;
            return false;
        }
        
        // Wait for connection with 5 second timeout
        fd_set write_fds;
        FD_ZERO(&write_fds);
        FD_SET(sock, &write_fds);
        
        struct timeval timeout;
        timeout.tv_sec = 5;
        timeout.tv_usec = 0;
        
        result = select(sock + 1, nullptr, &write_fds, nullptr, &timeout);
        
        if (result <= 0) {
            // Timeout or error
            std::cerr << "Connection timeout to " << ip << ":" << port << std::endl;
            close(sock);
            sock = -1;
            return false;
        }
        
        // Check if connection succeeded
        int error = 0;
        socklen_t len = sizeof(error);
        getsockopt(sock, SOL_SOCKET, SO_ERROR, &error, &len);
        
        if (error != 0) {
            std::cerr << "Failed to connect to " << ip << ":" << port << " - " << strerror(error) << std::endl;
            close(sock);
            sock = -1;
            return false;
        }
    }
    
    // Set socket back to blocking mode
    fcntl(sock, F_SETFL, flags);

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

void ClientConnection::adminLogin(const std::string& password) {
    MsgAdminLoginReq req{};
    strncpy(req.password, password.c_str(), sizeof(req.password) - 1);
    send_msg(sock, MSG_ADMIN_LOGIN_REQ, &req, sizeof(req));
}

void ClientConnection::createRoom() {
    send_msg(sock, MSG_CREATE_ROOM_REQ, nullptr, 0);
}

void ClientConnection::joinRoom(uint32_t roomId) {
    MsgGameIdReq req{ roomId };
    send_msg(sock, MSG_JOIN_ROOM_REQ, &req, sizeof(req));
}
void ClientConnection::startGame(uint32_t roomId) {
	MsgGameIdReq req{ roomId };
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

void ClientConnection::adminListGames() {
    send_msg(sock, MSG_ADMIN_LIST_GAMES_REQ, nullptr, 0);
}

void ClientConnection::adminListUsers() {
    send_msg(sock, MSG_ADMIN_LIST_USERS_REQ, nullptr, 0);
}

void ClientConnection::adminGetGameDetails(uint32_t gameId) {
    MsgGameIdReq req{ gameId };
    send_msg(sock, MSG_ADMIN_GAME_DETAILS_REQ, &req, sizeof(req));
}

void ClientConnection::adminTerminateGame(uint32_t gameId) {
    MsgGameIdReq req{ gameId };
    send_msg(sock, MSG_ADMIN_TERMINATE_GAME, &req, sizeof(req));
}

void ClientConnection::recvLoop() {
    char buffer[4096];

    while (running) {
        MsgHeader hdr;
        int r = recv_msg(sock, hdr, buffer, sizeof(buffer));
        if (r <= 0) {
            if (onServerShutdown) onServerShutdown();
            if (onError) onError("Disconnected from server");
            running = false;
            break;
        }
        handleMessage(hdr, buffer);
    }
}

void ClientConnection::handleMessage(const MsgHeader& hdr, char* payload) {
    switch (hdr.type) {
		case MSG_GAME_STATE: {
			MsgGameState copy = *reinterpret_cast<MsgGameState*>(payload);
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
        case MSG_ADMIN_PASSWORD_REQUIRED:
            if (onAdminPasswordRequired) onAdminPasswordRequired();
            break;
        case MSG_ADMIN_LOGIN_OK:
            if (onAdminLoginOk) onAdminLoginOk();
            break;
        case MSG_ADMIN_LOGIN_FAIL:
            if (onAdminLoginFail) onAdminLoginFail();
            break;
        case MSG_ADMIN_GAMES_LIST: {
            MsgAdminGamesList list = *reinterpret_cast<MsgAdminGamesList*>(payload);
            if (onAdminGamesList) onAdminGamesList(list);
            break;
        }
        case MSG_ADMIN_USERS_LIST: {
            MsgAdminUsersList list = *reinterpret_cast<MsgAdminUsersList*>(payload);
            if (onAdminUsersList) onAdminUsersList(list);
            break;
        }
        case MSG_ADMIN_GAME_DETAILS: {
            MsgAdminGameDetails details = *reinterpret_cast<MsgAdminGameDetails*>(payload);
            if (onAdminGameDetails) onAdminGameDetails(details);
            break;
        }
        case MSG_ADMIN_TERMINATE_OK:
            if (onAdminTerminateOk) onAdminTerminateOk();
            break;
        case MSG_ADMIN_TERMINATE_FAIL:
            if (onAdminTerminateFail) onAdminTerminateFail();
            break;
		case MSG_LOBBY_STATE: {
			MsgLobbyState copy = *reinterpret_cast<MsgLobbyState*>(payload);
			{
				std::lock_guard<std::mutex> lock(lobbyMutex);
				lastLobbyState = copy;
			}
			if (onLobbyState)
				onLobbyState(copy);
			break;
		}
        case MSG_CREATE_ROOM_OK: {
            MsgRoomInfo info = *reinterpret_cast<MsgRoomInfo*>(payload);
            std::vector<std::string> players;
            for (uint8_t i = 0; i < info.players_count && i < 8; ++i) {
                players.emplace_back(info.players[i]);
            }
            if (onCreateRoomOk)
                onCreateRoomOk(info.game_id, info.players_count, std::string(info.owner), players);
            break;
        }
        case MSG_JOIN_ROOM_OK: {
            MsgRoomInfo info = *reinterpret_cast<MsgRoomInfo*>(payload);
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
		case MSG_GAME_RESULTS: {
			MsgGameResults results = *reinterpret_cast<MsgGameResults*>(payload);
			if (onGameResults) onGameResults(results);
			break;
		}
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
			if (onServerShutdown) onServerShutdown();
			if (onError) onError("Server is shutting down");
			running = false;
			break;
		case MSG_EXIT_ROOM_OK:
			if (onExitRoomOk) onExitRoomOk();
			break;
		case MSG_EXIT_ROOM_FAIL:
			if (onExitRoomFail) onExitRoomFail();
			break;
		case MSG_CREATE_ROOM_FAIL:
			if (onCreateRoomFail) onCreateRoomFail();
			break;
        default:
			// Nieznany typ wiadomości
			break;
    }
}
