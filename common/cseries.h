#ifndef CSERIES_H
#define CSERIES_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include "win_compat.h"
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

// Constants
#define MAX_USERNAME_LENGTH 32
#define MAX_PATH_LENGTH 256

// Function declarations
#ifdef _WIN32
size_t myth_strnlen(const char* s, size_t maxlen);
char* myth_strupr(char* s);
char* myth_strnupr(char* s, size_t n);
char* myth_strlwr(char* s);
char* myth_strnlwr(char* s, size_t n);
#else
size_t strnlen(const char* s, size_t maxlen);
char* strupr(char* s);
char* strnupr(char* s, size_t n);
char* strlwr(char* s);
char* strnlwr(char* s, size_t n);
#endif

#endif /* CSERIES_H */