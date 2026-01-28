#include "services.h"
#include "service_log.h"
#include <stdio.h>
#include <string.h>

#include <Arduino.h>

static service_status_t st = {
    .state = SERVICE_STOPPED,
    .log = "GSM idle"
};

static int gsm_start(service_status_t *out){

    // GSM Initalization
    SERVICE_LOG(&st, "[INFO] GSM starting...\n");

    // Add GSM initialization code here   

    st.state = SERVICE_RUNNING;
    st.log[0] = '\0';
    SERVICE_LOG(&st, "[ OK ] GSM started successfully");
    *out = st;

    return 0;
}

static int gsm_stop(service_status_t *out){
    // Add GSM de-initialization code here

    st.state = SERVICE_STOPPED;
    st.log[0] = '\0';
    SERVICE_LOG(&st, "[ OK ] GSM stopped");
    *out = st;
    return 0;
}

static void gsm_status(service_status_t *out){
    *out = st;
}

service_t service_gsm = {
    .name   = "gsm",
    .start  = gsm_start,
    .stop   = gsm_stop,
    .status = gsm_status,
};