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

#define DEFAULT_SSID "MotorolaGearSolid"
#define DEFAULT_PASS "batpesho4"



static int wifi_start(service_status_t *out){

    // WiFi Initalization
    WiFi.mode(WIFI_STA);

        // Add code that reads from flash the SSID and password
    WiFi.begin(DEFAULT_SSID, DEFAULT_PASS);
    WiFi.setAutoReconnect(true);
    WiFi.setHostname("SOP-PHONE");
    
    
    st.state = SERVICE_RUNNING;
    st.log[0] = '\0';
    SERVICE_LOG(&st, "[INFO] WiFi started, connecting...");

    *out = st;
    return 0;
}

static int wifi_stop(service_status_t *out){
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);

    st.state = SERVICE_STOPPED;
    st.log[0] = '\0';
    SERVICE_LOG(&st, "[ OK ] WiFi stopped");
    *out = st;
    return 0;
}

static void wifi_status(service_status_t *out){
    if (st.state == SERVICE_RUNNING) {
        st.log[0] = '\0';
        if (WiFi.status() == WL_CONNECTED) {
            SERVICE_LOG(&st, "[ OK ] Connected: %s", WiFi.localIP().toString().c_str());
        } else {
            SERVICE_LOG(&st, "[INFO] Connecting...");
        }
    }
    *out = st;
}

service_t service_wifi = {
    .name   = "wifi",
    .start  = wifi_start,
    .stop   = wifi_stop,
    .status = wifi_status,
};
