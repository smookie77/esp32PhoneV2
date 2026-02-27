#include "services.h"
#include "service_log.h"
#include <stdio.h>
#include <string.h>

#include <Arduino.h>
#include "keypad_driver.h"

KeypadDriver keypad(0x20); // Address 0x20 for TCA9555

static service_status_t st = {
    .state = SERVICE_STOPPED,
    .log = "Keyboard idle"
};

static int keyboard_start(service_status_t *out){

    // Keyboard Initalization
    SERVICE_LOG(&st, "[INFO] Keyboard starting...\n");

    if (keypad.begin()) {
        SERVICE_LOG(&st, "[ OK ] Keypad (TCA9555) initialized!");
        st.state = SERVICE_RUNNING;
        st.log[0] = '\0';
        *out = st;
        return 0;
    } else {
        SERVICE_LOG(&st, "[ERR ] Keypad NOT found! Check wiring.");
        st.state = SERVICE_ERROR;
        *out = st;
        return -1;
    }
}

static int keyboard_stop(service_status_t *out){
    // Add keyboard de-initialization code here

    st.state = SERVICE_STOPPED;
    st.log[0] = '\0';
    SERVICE_LOG(&st, "[ OK ] Keyboard stopped");
    *out = st;
    return 0;
}

static void keyboard_status(service_status_t *out){
    *out = st;
}

service_t service_keyboard = {
    .name   = "keyboard",
    .start  = keyboard_start,
    .stop   = keyboard_stop,
    .status = keyboard_status,
};