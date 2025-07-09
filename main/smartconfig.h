/**
 * 
 */

#pragma once

#include <stdio.h>

#include "esp_err.h"


#ifdef __cplusplus
extern "C" {
#endif

enum wifi_state {
    WIFI_CONNECTING = BIT0,
    WIFI_CONNECTED = BIT1,
    SMARTCONFIG_START = BIT2,
    SMARTCONFIG_END = BIT3,
};

class WiFiConnect {
public:
    EventGroupHandle_t wifi_event_group_;

public:
    esp_err_t init();
};


#ifdef __cplusplus
}
#endif

