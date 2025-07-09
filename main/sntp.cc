/**
 * @file sntp.cc
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2025-07-08
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include "sntp.h"
#include "esp_sntp.h"
#include "esp_netif_sntp.h"

static const char TAG[] = "time";
static const char SNTP_SERVER[] = "pool.ntp.org";

static void time_sync_cb(struct timeval *tv)
{
    ESP_LOGI(TAG, "Notification of a time synchronization event");
}

esp_err_t SyncTime::get_local_time(void)
{
    esp_err_t ret = ESP_OK;

    time_t now;
    char strftime_buf[64];

    time(&now);

    setenv("TZ", "CST-8", 1);
    tzset();
    localtime_r(&now, &datetime_);
    strftime(strftime_buf, sizeof(strftime_buf), "%c", &datetime_);
    ESP_LOGI(TAG, "The current date/time in Shanghai is: %s", strftime_buf);

    return ret;
}

esp_err_t SyncTime::sync(void)
{
    esp_err_t ret = ESP_OK;
    esp_sntp_config_t sntp_config = ESP_NETIF_SNTP_DEFAULT_CONFIG(SNTP_SERVER);

    sntp_config.sync_cb = time_sync_cb;
    esp_netif_sntp_init(&sntp_config);

    return ret;
}

