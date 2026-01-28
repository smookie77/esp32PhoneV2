#include <servicemgr.h>

#include <string.h>


/* ------------------------------ */
/* ADD ADDITIONAL SERVICES HERE:*/
/* 
1. Create an appropriatelly named file
for the service
2. Add "extern service_t yourservice;" below
3. Add the service to the array of services
*/ 
extern service_t service_wifi;
extern service_t service_keyboard;
extern service_t service_gsm;
// extern service_t service_net;

static service_t *services[] = {
    &service_wifi,
    &service_keyboard,
    &service_gsm,
//     &service_net,
};
/* ------------------------------ */

#define SERVICE_COUNT (sizeof(services) / sizeof(services[0]))



static service_status_t statuses[SERVICE_COUNT];

static int find_service(const char *name){
    for (int i = 0; i < SERVICE_COUNT; i++) {
        if (strcmp(services[i]->name, name) == 0)
            return i;
    }
    return -1;
}

int service_start(const char *name){
    int i = find_service(name);
    if (i < 0) return -1;

    return services[i]->start(&statuses[i]);
}

int service_stop(const char *name){
    int i = find_service(name);
    if (i < 0) return -1;

    return services[i]->stop(&statuses[i]);
}

int service_get_status(const char *name, service_status_t *out){
    int i = find_service(name);
    if (i < 0) return -1;

    services[i]->status(&statuses[i]);
    *out = statuses[i];
    return 0;
}
