#pragma once

#include <cstdint>
#include "protocol.h"

int send_msg(int fd, uint16_t type, const void* payload, uint16_t length);
int recv_msg(int fd, MsgHeader& hdr, void* buffer, uint16_t buffer_size);
