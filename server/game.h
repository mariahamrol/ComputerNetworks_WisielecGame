#ifndef GAME_H
#define GAME_H

#include <stdint.h>
#include "../include/messages.h"

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
} Game;

#endif
