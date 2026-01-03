#ifndef GAME_H
#define GAME_H

#include <unordered_map>
#include <vector>
#include <chrono>
#include "../include/messages.h"

typedef struct {
    char letter;
    std::vector<int> players; 
    std::chrono::steady_clock::time_point reveal_at;
} PendingGuess;

typedef struct {
    int id;
    int players[MAX_PLAYERS];
    int player_count;
	char owner[MAX_NICK_LEN];
    char word[MAX_WORD_LEN];
	uint8_t word_length;
	char word_guessed[MAX_WORD_LEN];
	char guessed_letters[ALPHABET_SIZE];
    int active;
	std::unordered_map<char, PendingGuess> pending;
} Game;

#endif
