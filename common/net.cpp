#include "net.hpp"
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>

static int write_all(int fd, const void* buf, size_t len) {
    const char* p = static_cast<const char*>(buf);
    while (len > 0) {
        ssize_t n = send(fd, p, len, 0);
        if (n < 0) {
            if (errno == EINTR) continue;
            if (errno == EAGAIN || errno == EWOULDBLOCK) return 0;
            return -1;
        }
        p += n;
        len -= n;
    }
    return 1;
}

static int read_all(int fd, void* buf, size_t len) {
    char* p = static_cast<char*>(buf);
    while (len > 0) {
        ssize_t n = recv(fd, p, len, 0);
        if (n == 0) return 0;
        if (n < 0) {
            if (errno == EINTR) continue;
            if (errno == EAGAIN || errno == EWOULDBLOCK) return 0;
            return -1;
        }
        p += n;
        len -= n;
    }
    return 1;
}

int send_msg(int fd, uint16_t type, const void* payload, uint16_t length) {
    MsgHeader hdr{type, length};

    if (write_all(fd, &hdr, sizeof(hdr)) != 1) return -1;
    if (length > 0 && payload)
        if (write_all(fd, payload, length) != 1) return -1;

    return 0;
}

int recv_msg(int fd, MsgHeader& hdr, void* buffer, uint16_t buffer_size) {
    int r = read_all(fd, &hdr, sizeof(MsgHeader));
    if (r <= 0) return r;

    if (hdr.length > buffer_size) return -1;

    if (hdr.length > 0) {
        r = read_all(fd, buffer, hdr.length);
        if (r <= 0) return r;
    }
    return 1;
}
