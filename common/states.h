#ifndef STATES_H
#define STATES_H

typedef enum {
    STATE_CONNECTED,
    STATE_LOGGING_IN,
    STATE_LOBBY,
    STATE_IN_ROOM,
    STATE_IN_GAME,
    STATE_ADMIN
} ClientState;

#endif
