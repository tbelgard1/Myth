#ifndef POSIX_COMPAT_H
#define POSIX_COMPAT_H

#ifdef _WIN32
#include <io.h>
#include <direct.h>
#include <process.h>
#include "win_compat.h"

// Map POSIX functions to Windows equivalents
#define access _access
#define chdir _chdir
#define getcwd _getcwd
#define mkdir(path, mode) _mkdir(path)
#define rmdir _rmdir
#define unlink _unlink
#define close _close
#define dup _dup
#define dup2 _dup2
#define fileno _fileno
#define isatty _isatty
#define lseek _lseek
#define open _open
#define read _read
#define write _write
#define pclose _pclose
#define popen _popen
#define getpid _getpid

// Sleep functions
#define sleep(x) Sleep((x) * 1000)
#define usleep(x) Sleep((x) / 1000)

// File mode constants
#define F_OK 0
#define X_OK 1
#define W_OK 2
#define R_OK 4

// Other POSIX constants
#define PATH_MAX MAX_PATH

#else
#include <unistd.h>
#endif

#endif // POSIX_COMPAT_H