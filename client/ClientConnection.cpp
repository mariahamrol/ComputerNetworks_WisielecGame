#include "ClientConnection.h"
#include "../common/net.hpp"

#include <unistd.h>
#include <arpa/inet.h>
#include <cstring>

ClientConnection::ClientConnection() {}

ClientConnection::~ClientConnection() {
    disconnect();
}

std::optional<MsgLobbyState> ClientConnection::getLastLobbyState() {
    std::lock_guard<std::mutex> lock(lobbyMutex);
    return lastLobbyState;
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

void ClientConnection::createGame() {
    send_msg(sock, MSG_CREATE_GAME_REQ, nullptr, 0);
}

void ClientConnection::joinGame(uint32_t id) {
    MsgJoinGameReq req{ id };
    send_msg(sock, MSG_JOIN_GAME_REQ, &req, sizeof(req));
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
        case MSG_CREATE_GAME_OK:
            if (onCreateGameOk) onCreateGameOk();
            break;
        case MSG_JOIN_GAME_OK:
            if (onJoinGameOk) onJoinGameOk();
            break;
        default:
            break;
    }
}
