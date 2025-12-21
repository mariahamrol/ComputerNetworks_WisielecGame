#pragma once
#include <string>
#include <vector>
#include <functional>
#include <thread>
#include <atomic>
#include <mutex>
#include <optional>

#include "../include/protocol.h"
#include "../include/messages.h"

class ClientConnection {
public:
    // --- callbacks (UI je podpinia) ---
    std::function<void()> onLoginOk;
    std::function<void()> onLoginTaken;
    std::function<void(const MsgLobbyState&)> onLobbyState;
    std::function<void(uint32_t gameId, uint8_t playersCount, const std::string &owner, const std::vector<std::string> &players)> onCreateRoomOk;
    std::function<void(uint32_t gameId, uint8_t playersCount, const std::string &owner, const std::vector<std::string> &players)> onJoinRoomOk;
	std::function<void()> onJoinRoomFail;
    std::function<void()> onStartGameOk;
    std::function<void()> onGuessLetterOk;
	std::function<void()> onGuessLetterFail;
	std::function<void(const MsgGameState&)> onGameState;
    std::function<void()> onStartGameFail;
    std::function<void(const std::string&)> onError;
	std::optional<MsgLobbyState> getLastLobbyState();
	std::optional<MsgGameState> getLastGameState();
	
    ClientConnection();
    ~ClientConnection();

    bool connectToServer(const std::string& ip, int port);
    void disconnect();

    // --- API dla GUI ---
    void login(const std::string& nick);
    void createRoom();
    void joinRoom(uint32_t roomId);
	void startGame(uint32_t roomId);
	void guessLetter(char letter);

private:
    int sock = -1;
    std::thread recvThread;
    std::atomic<bool> running{false};
	std::mutex lobbyMutex;
    std::optional<MsgLobbyState> lastLobbyState;
	std::mutex gameStateMutex;
	std::optional<MsgGameState> lastGameState;

    void recvLoop();
    void handleMessage(MsgHeader& hdr, char* payload);


};
