#include "service_log.h"
#include <stdarg.h>

void service_log_write(service_status_t *st, const char *fmt, ...){
    va_list ap;
    size_t len = strlen(st->log);

    if (len >= SERVICE_LOG_SIZE - 2)
        return;

    va_start(ap, fmt);
    vsnprintf(st->log + len,
              SERVICE_LOG_SIZE - len,
              fmt,
              ap);
    va_end(ap);
}
