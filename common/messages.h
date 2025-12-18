#ifndef MESSAGES_H
#define MESSAGES_H

#include <stdint.h>

#define MAX_NICK_LEN 32
#define MAX_WORD_LEN 64
#define MAX_GAMES 32

typedef struct {
    char nick[MAX_NICK_LEN];
} MsgLoginReq;

typedef struct {
    uint32_t game_id;
} MsgJoinGameReq;

typedef struct {
    char letter;
} MsgGuessLetterReq;

// Wiadomość od serwera, zawierająca listę dostępnych gier
typedef struct {
    uint32_t game_id;
    uint8_t players_count;
} GameInfo;

typedef struct {
    uint32_t games_count;
    GameInfo games[MAX_GAMES];
} MsgLobbyState;

typedef struct {
    uint32_t game_id;
    uint8_t players_count;
} LobbyGameInfo;

/* ===== GAME STATE ===== */

typedef struct {
    char nick[MAX_NICK_LEN];
    uint8_t lives;
    uint16_t points;
} PlayerState;

typedef struct {
    uint8_t player_count;
    PlayerState players[];
} MsgGameState;

typedef struct {
    char letter;
    uint8_t correct;
    uint8_t count;
    uint8_t positions[];
} MsgGuessResult;

#endif
