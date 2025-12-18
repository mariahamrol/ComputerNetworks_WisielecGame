#pragma once
#include <string>
#include <functional>
#include <thread>
#include <atomic>
#include <mutex>
#include <optional>

#include "../common/protocol.h"
#include "../common/messages.h"

class ClientConnection {
public:
    // --- callbacks (UI je podpinia) ---
    std::function<void()> onLoginOk;
    std::function<void()> onLoginTaken;
    std::function<void(const MsgLobbyState&)> onLobbyState;
    std::function<void()> onCreateGameOk;
    std::function<void()> onJoinGameOk;
    std::function<void(const std::string&)> onError;
	std::optional<MsgLobbyState> getLastLobbyState();

    ClientConnection();
    ~ClientConnection();

    bool connectToServer(const std::string& ip, int port);
    void disconnect();

    // --- API dla GUI ---
    void login(const std::string& nick);
    void createGame();
    void joinGame(uint32_t gameId);

private:
    int sock = -1;
    std::thread recvThread;
    std::atomic<bool> running{false};
	std::mutex lobbyMutex;
    std::optional<MsgLobbyState> lastLobbyState;

    void recvLoop();
    void handleMessage(MsgHeader& hdr, char* payload);


};
