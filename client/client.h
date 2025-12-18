#ifndef SERVER_CLIENT_H
#define SERVER_CLIENT_H

#include <vector>
#include "../common/states.h"
#include "../common/messages.h"


struct Client {
    int fd;
    ClientState state;
    char nick[MAX_NICK_LEN];
    int game_id;
	int is_owner;
    int active;
    std::vector<char> buffer;
};

#endif
