#ifndef LOGGING_H
#define LOGGING_H
#include <stdio.h>
#include <errno.h>
#include <string.h>

#define log_debug(fmt, ...) fprintf(stdout, "[DEBUG] " fmt "\n", ##__VA_ARGS__); fflush(stdout)
#define log_error(fmt, ...) fprintf(stderr, "[ERROR] " fmt "\n", ##__VA_ARGS__); fflush(stderr)
#define log_warn(fmt, ...) fprintf(stderr, "[WARN] " fmt "\n", ##__VA_ARGS__); fflush(stderr)

#endif // LOGGING_H
