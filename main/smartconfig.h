/**
 * 
 */

#pragma once

#include "string.h"
#include "stdlib.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_eap_client.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_smartconfig.h"
#include "esp_mac.h"

#ifdef __cplusplus
extern "C" {
#endif

void init_wifi(void);

#ifdef __cplusplus
}
#endif
