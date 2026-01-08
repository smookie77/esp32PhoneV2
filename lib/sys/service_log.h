#pragma once
#include "services.h"
#include <stdio.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

void service_log_write(service_status_t *st, const char *fmt, ...);

#define SERVICE_LOG(st, fmt, ...) \
    service_log_write((st), fmt, ##__VA_ARGS__)

#ifdef __cplusplus
}
#endif
