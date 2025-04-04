#include "win_compat.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>

#ifdef _WIN32

int fcntl(int fd, int cmd, ...) {
    // Windows doesn't support fcntl, so we'll return -1 for now
    return -1;
}

int gettimeofday(struct timeval* tv, void* tz) {
    if (tv) {
        FILETIME ft;
        GetSystemTimeAsFileTime(&ft);
        ULARGE_INTEGER uli;
        uli.LowPart = ft.dwLowDateTime;
        uli.HighPart = ft.dwHighDateTime;
        tv->tv_sec = (long)(uli.QuadPart / 10000000ULL - 11644473600ULL);
        tv->tv_usec = (long)(uli.QuadPart % 10000000ULL / 10);
    }
    return 0;
}

int usleep(unsigned int usec) {
    Sleep(usec / 1000);
    return 0;
}

int pipe(int fds[2]) {
    // Windows doesn't support pipes in the same way as POSIX
    return -1;
}

int close(int fd) {
    return closesocket(fd);
}

int read(int fd, void* buf, size_t count) {
    return recv(fd, buf, (int)count, 0);
}

int write(int fd, const void* buf, size_t count) {
    return send(fd, buf, (int)count, 0);
}

int select(int nfds, fd_set* readfds, fd_set* writefds, fd_set* exceptfds, struct timeval* timeout) {
    return select(nfds, readfds, writefds, exceptfds, timeout);
}

#endif /* _WIN32 */