#include "services.h"
#include "service_log.h"
#include <stdio.h>
#include <string.h>

#include <Arduino.h>
#include <WiFi.h>


static service_status_t st = {
    .state = SERVICE_STOPPED,
    .log = "WiFi idle"
};

#define DEFAULT_SSID    "MotorolaGearSolid"
#define DEFUALT_PASS    "batpesho4"



static int wifi_start(service_status_t *out){

    // WiFi Initalization
    WiFi.mode(WIFI_STA);

        // Add code that reads from flash the SSID and password
    WiFi.setHostname("SOP-PHONE");
    WiFi.begin(DEFAULT_SSID, DEFUALT_PASS);
    WiFi.setAutoReconnect(true);
    SERVICE_LOG(&st, "[INFO] WiFi conecting...\n");
    
    if(WiFi.status() != WL_CONNECTED){
        st.state = SERVICE_RUNNING;
        st.log[0] = '\0';
        SERVICE_LOG(&st, "[ OK ] WiFi started successfully");
        *out = st;
    }

    return 0;
}

static int wifi_stop(service_status_t *out){
    WiFi.disconnect();

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
