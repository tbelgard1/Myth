#ifndef WIN_COMPAT_H
#define WIN_COMPAT_H

// Prevent winsock.h from being included by windows.h
#define _WINSOCKAPI_
#define _WINSOCK2API_

// Include winsock2.h before windows.h
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>

// Additional Windows compatibility definitions
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _USE_WINSOCK2
#define _WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT 0x0601

// Windows-specific type definitions
typedef int socklen_t;
typedef SOCKET socket_t;

// Windows-specific function declarations
#define strcasecmp _stricmp
#define strncasecmp _strnicmp
#define strdup _strdup
#define strtok_r strtok_s
#define strerror_r(errno, buf, len) strerror_s(buf, len, errno)

// Windows-specific error codes
#ifndef EINTR
#define EINTR WSAEINTR
#endif
#ifndef EAGAIN
#define EAGAIN WSAEWOULDBLOCK
#endif

// Forward declarations for socket structures
struct sockaddr;
struct sockaddr_in;
struct sockaddr_in6;
struct sockaddr_storage;

// Define basic socket types if not already defined
#ifndef SOCKET
#define SOCKET unsigned int
#endif

#ifndef INVALID_SOCKET
#define INVALID_SOCKET (SOCKET)(~0)
#endif

#ifndef SOCKET_ERROR
#define SOCKET_ERROR (-1)
#endif

// Define missing types from ws2ipdef.h
typedef unsigned short ADDRESS_FAMILY;
typedef unsigned long SCOPE_ID;
typedef struct in_addr IN_ADDR;
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr_in6 SOCKADDR_IN6;
typedef struct sockaddr_storage SOCKADDR_STORAGE;

// Define missing constants
#define IN_PKTINFO 19
#define scopeid_unspecified 0
#define in4addr_any {0}
#define in4addr_loopback {0x0100007f}
#define in4addr_broadcast {0xffffffff}
#define in4addr_allnodesonlink {0xe0000001}
#define in4addr_allroutersonlink {0xe0000002}
#define in4addr_alligmpv3routersonlink {0xe0000016}
#define in4addr_allteredohostsonlink {0xe00000fb}
#define in4addr_linklocalprefix {0xa9fe0000}
#define in4addr_multicastprefix {0xe0000000}

#ifdef __cplusplus
extern "C" {
#endif

	// Add any additional Windows compatibility functions here

#ifdef __cplusplus
}
#endif

#endif // WIN_COMPAT_H