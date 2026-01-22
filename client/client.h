#ifndef SERVER_CLIENT_H
#define SERVER_CLIENT_H

#include <vector>
#include <array>
#include <chrono>
#include "../include/states.h"

#define ALPHABET_SIZE 32
#define MAX_NICK_LEN 32
struct Client {
    int fd;
    ClientState state;
    char nick[MAX_NICK_LEN];
    int game_id;
	int is_owner;
    int is_active;
	char guessed_letters[ALPHABET_SIZE];
	uint8_t lives;
    uint16_t points;
    std::vector<char> buffer;
	std::chrono::steady_clock::time_point last_activity;
    std::chrono::steady_clock::time_point last_ping_sent;
};

#endif
