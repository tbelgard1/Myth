#include "cseries.h"
#include <stdlib.h>
#include <string.h>

// Environment variables structure
typedef struct {
    char* name;
    char* value;
} env_var_t;

// Global environment variables array
static env_var_t* env_vars = NULL;
static size_t env_vars_count = 0;
static size_t env_vars_capacity = 0;

// Initialize environment
int environment_init(void) {
    env_vars_capacity = 16;
    env_vars = (env_var_t*)malloc(env_vars_capacity * sizeof(env_var_t));
    if (!env_vars) return -1;

    env_vars_count = 0;
    return 0;
}

// Clean up environment
void environment_cleanup(void) {
    if (env_vars) {
        for (size_t i = 0; i < env_vars_count; i++) {
            free(env_vars[i].name);
            free(env_vars[i].value);
        }
        free(env_vars);
        env_vars = NULL;
    }
    env_vars_count = 0;
    env_vars_capacity = 0;
}

// Set environment variable
int environment_set(const char* name, const char* value) {
    if (!name || !value) return -1;

    // Check if variable already exists
    for (size_t i = 0; i < env_vars_count; i++) {
        if (strcmp(env_vars[i].name, name) == 0) {
            free(env_vars[i].value);
            env_vars[i].value = strdup(value);
            return 0;
        }
    }

    // Resize array if needed
    if (env_vars_count >= env_vars_capacity) {
        size_t new_capacity = env_vars_capacity * 2;
        env_var_t* new_vars = (env_var_t*)realloc(env_vars, new_capacity * sizeof(env_var_t));
        if (!new_vars) return -1;
        env_vars = new_vars;
        env_vars_capacity = new_capacity;
    }

    // Add new variable
    env_vars[env_vars_count].name = strdup(name);
    env_vars[env_vars_count].value = strdup(value);
    if (!env_vars[env_vars_count].name || !env_vars[env_vars_count].value) {
        free(env_vars[env_vars_count].name);
        free(env_vars[env_vars_count].value);
        return -1;
    }

    env_vars_count++;
    return 0;
}

// Get environment variable
const char* environment_get(const char* name) {
    if (!name) return NULL;

    for (size_t i = 0; i < env_vars_count; i++) {
        if (strcmp(env_vars[i].name, name) == 0) {
            return env_vars[i].value;
        }
    }

    return NULL;
}

// Unset environment variable
int environment_unset(const char* name) {
    if (!name) return -1;

    for (size_t i = 0; i < env_vars_count; i++) {
        if (strcmp(env_vars[i].name, name) == 0) {
            free(env_vars[i].name);
            free(env_vars[i].value);

            // Shift remaining variables
            for (size_t j = i; j < env_vars_count - 1; j++) {
                env_vars[j] = env_vars[j + 1];
            }

            env_vars_count--;
            return 0;
        }
    }

    return -1;
}