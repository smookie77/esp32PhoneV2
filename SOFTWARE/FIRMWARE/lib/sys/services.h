#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#define SERVICE_LOG_SIZE 128

typedef enum {
    SERVICE_STOPPED = 0,
    SERVICE_RUNNING,
    SERVICE_ERROR
} service_state_t;

typedef struct {
    service_state_t state;
    char log[SERVICE_LOG_SIZE];
} service_status_t;


typedef int  (*service_function_t)(service_status_t *);
typedef void (*service_status_function_t)(service_status_t *);

typedef struct service {
    const char *name;

    service_function_t start;
    service_function_t stop;
    service_status_function_t status;
} service_t;

#ifdef __cplusplus
}
#endif
