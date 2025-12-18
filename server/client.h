#ifndef SERVER_CLIENT_H
#define SERVER_CLIENT_H

#include <vector>
#include "../common/states.h"

struct Client {
    int fd;
    ClientState state;
    char nick[32];
    int game_id;
    int active;
    std::vector<char> buffer;
};

#endif
