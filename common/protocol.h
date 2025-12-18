#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdint.h>

#pragma pack(push,1)
typedef struct {
    uint16_t type;
    uint16_t length;
} MsgHeader;
#pragma pack(pop)

typedef enum {
    /* LOGIN */
    MSG_LOGIN_REQ = 1,
    MSG_LOGIN_OK,
    MSG_LOGIN_TAKEN,
    MSG_LOGIN_ERROR,

    /* LOBBY */
    MSG_LOBBY_STATE,
    MSG_CREATE_GAME_REQ,
    MSG_CREATE_GAME_OK,
    MSG_CREATE_GAME_FAIL,
    MSG_JOIN_GAME_REQ,
    MSG_JOIN_GAME_OK,
    MSG_JOIN_GAME_FAIL,

    /* ROOM */
    MSG_ROOM_STATE,
    MSG_START_GAME_REQ,
    MSG_START_GAME_FAIL,

    /* GAME */
    MSG_GAME_START,
    MSG_GUESS_LETTER_REQ,
    MSG_GUESS_RESULT,
    MSG_GAME_STATE,
    MSG_GAME_END,

    /* ADMIN */
    MSG_ADMIN_LOGIN_REQ,
    MSG_ADMIN_LOGIN_OK,
    MSG_ADMIN_LOGIN_FAIL,
    MSG_ADMIN_GAMES_LIST,
    MSG_ADMIN_TERMINATE_GAME,

    MSG_ERROR
} MsgType;

#endif
