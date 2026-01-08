#include "services.h"
#include "service_log.h"
#include <stdio.h>
#include <string.h>

/* Вътрешно състояние */
static service_status_t st = {
    .state = SERVICE_STOPPED,
    .log = "WiFi idle"
};

static int wifi_start(service_status_t *out){
    st.state = SERVICE_RUNNING;
    st.log[0] = '\0';
    SERVICE_LOG(&st, "[ OK ] WiFi started successfully");
    *out = st;
    return 0;
}

static int wifi_stop(service_status_t *out){
    st.state = SERVICE_STOPPED;
    st.log[0] = '\0';
    SERVICE_LOG(&st, "[ OK ] WiFi stopped");
    *out = st;
    return 0;
}

static void wifi_status(service_status_t *out){
    *out = st;
}

service_t service_wifi = {
    .name   = "wifi",
    .start  = wifi_start,
    .stop   = wifi_stop,
    .status = wifi_status,
};