#include "cseries.h"

#ifdef _WIN32
size_t myth_strnlen(const char* s, size_t maxlen) {
    const char* p = s;
    while (maxlen-- && *p) p++;
    return p - s;
}

char* myth_strupr(char* s) {
    char* p = s;
    while (*p) {
        *p = toupper(*p);
        p++;
    }
    return s;
}

char* myth_strnupr(char* s, size_t n) {
    char* p = s;
    while (n-- && *p) {
        *p = toupper(*p);
        p++;
    }
    return s;
}

char* myth_strlwr(char* s) {
    char* p = s;
    while (*p) {
        *p = tolower(*p);
        p++;
    }
    return s;
}

char* myth_strnlwr(char* s, size_t n) {
    char* p = s;
    while (n-- && *p) {
        *p = tolower(*p);
        p++;
    }
    return s;
}
#else
// Original implementations for non-Windows platforms
size_t strnlen(const char* s, size_t maxlen) {
    const char* p = s;
    while (maxlen-- && *p) p++;
    return p - s;
}

char* strupr(char* s) {
    char* p = s;
    while (*p) {
        *p = toupper(*p);
        p++;
    }
    return s;
}

char* strnupr(char* s, size_t n) {
    char* p = s;
    while (n-- && *p) {
        *p = toupper(*p);
        p++;
    }
    return s;
}

char* strlwr(char* s) {
    char* p = s;
    while (*p) {
        *p = tolower(*p);
        p++;
    }
    return s;
}

char* strnlwr(char* s, size_t n) {
    char* p = s;
    while (n-- && *p) {
        *p = tolower(*p);
        p++;
    }
    return s;
}
#endif