#include "services.h"
#include "service_log.h"
#include <stdio.h>
#include <string.h>

#include <Arduino.h>

static service_status_t st = {
    .state = SERVICE_STOPPED,
    .log = "Display idle"
};

static int display_start(service_status_t *out){

    // Display Initalization
    SERVICE_LOG(&st, "[INFO] Display starting...\n");

    // Add display initialization code here   

    st.state = SERVICE_RUNNING;
    st.log[0] = '\0';
    SERVICE_LOG(&st, "[ OK ] Display started successfully");
    *out = st;

    return 0;
}

static int display_stop(service_status_t *out){
    // Add display de-initialization code here

    st.state = SERVICE_STOPPED;
    st.log[0] = '\0';
    SERVICE_LOG(&st, "[ OK ] Display stopped");
    *out = st;
    return 0;
}

static void display_status(service_status_t *out){
    *out = st;
}

service_t service_display = {
    .name   = "display",
    .start  = display_start,
    .stop   = display_stop,
    .status = display_status,
};