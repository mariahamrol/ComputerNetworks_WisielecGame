#ifndef SERVER_CLIENT_H
#define SERVER_CLIENT_H

#include "../common/states.h"

typedef struct {
    int fd;
    ClientState state;
    char nick[32];
    int game_id;
    int active;
} Client;

#endif
