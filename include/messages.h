#ifndef MESSAGES_H
#define MESSAGES_H

#include <stdint.h>
#include "../client/client.h"

#define MAX_NICK_LEN 32
#define MAX_WORD_LEN 64
#define MAX_GAMES 32
#define MAX_LIVES 8
#define ALPHABET_SIZE 32




typedef struct {
    char nick[MAX_NICK_LEN];
} MsgLoginReq;

typedef struct {
    char password[32];
} MsgAdminLoginReq;

typedef struct {
    uint32_t game_id;
}  MsgGameIdReq;

// Sent by server to confirm room creation / join with basic room info
typedef struct {
    uint32_t game_id;
    uint8_t players_count;
    char owner[MAX_NICK_LEN];
    char players[8][MAX_NICK_LEN];
} MsgRoomInfo;

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

/* ===== ADMIN MESSAGES ===== */
typedef struct {
    uint32_t game_id;
    uint8_t players_count;
} AdminGameInfo;

typedef struct {
    uint32_t games_count;
    AdminGameInfo games[MAX_GAMES];
} MsgAdminGamesList;

/* ===== GAME STATE ===== */

typedef struct {
    char nick[MAX_NICK_LEN];
	char guessed_letters[ALPHABET_SIZE];
	int is_owner;
    bool is_active;
    uint8_t lives;
    uint16_t points;
} NetPlayerState;


typedef struct {
	uint32_t game_id;
	char word[MAX_WORD_LEN];
	char guessed_letters[ALPHABET_SIZE];
    uint8_t word_length;
	uint8_t player_count;
    NetPlayerState players[8];
} MsgGameState;



#endif
