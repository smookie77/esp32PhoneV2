#pragma once
#include "services.h"

#ifdef __cplusplus
extern "C" {
#endif

int service_start(const char *name);
int service_stop(const char *name);
int service_get_status(const char *name, service_status_t *out);

#ifdef __cplusplus
}
#endif