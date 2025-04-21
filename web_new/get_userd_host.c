#include <stdlib.h>
#include <string.h>

// Returns the userd host from the environment, or "127.0.0.1" if not set
const char *get_userd_host(void) {
    const char *env = getenv("USERD_HOST");
    if(env && strlen(env) > 0) {
        return env;
    }
    return "127.0.0.1";
}
