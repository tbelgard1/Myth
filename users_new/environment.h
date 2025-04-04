#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include "cseries.h"

#ifdef __cplusplus
extern "C" {
#endif

	// Function declarations
	int environment_init(void);
	void environment_cleanup(void);
	int environment_set(const char* name, const char* value);
	const char* environment_get(const char* name);
	int environment_unset(const char* name);

#ifdef __cplusplus
}
#endif

#endif /* ENVIRONMENT_H */