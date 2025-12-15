#ifndef GAME_H
#define GAME_H

#include <stdint.h>

#define MAX_PLAYERS 8

typedef struct {
    int id;
    int players[MAX_PLAYERS];
    int player_count;

    char word[64];
    uint8_t guessed[26];

    int started;
} Game;

#endif
