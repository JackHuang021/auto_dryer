/**
 * 
 */

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
#include "smartconfig.h"


static const char *tag = "wifi";
static bool smartconfig_run = false;

static esp_err_t save_wifi_credentials(const char *ssid, const char *password)
{
    esp_err_t ret = ESP_OK;
    nvs_handle_t nvs_handle;

    ret = nvs_open("storage", NVS_READWRITE, &nvs_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(tag, "Error (%s) opening NVS handle!", esp_err_to_name(ret));
        return ret;
    }

    ret = nvs_set_str(nvs_handle, "ssid", ssid);
    if (ret != ESP_OK) {
        ESP_LOGE(tag, "set ssid failed, return %s", esp_err_to_name(ret));
        goto out;
    }

    ret = nvs_set_str(nvs_handle, "password", password);
    if (ret != ESP_OK) {
        ESP_LOGE(tag, "set password failed, return %s", esp_err_to_name(ret));
        goto out;
    }

    ret = nvs_commit(nvs_handle);

    if (ret == ESP_OK)
        ESP_LOGI(tag, "WiFi credentials saved: SSID=%s, PASSWORD=%s", ssid, password);
    else
        ESP_LOGE(tag, "save wifi info failed, return %s", esp_err_to_name(ret));
out:
    nvs_close(nvs_handle);
    return ret;
}

static bool read_wifi_credentials(char *ssid, char *password)
{
    esp_err_t ret = ESP_OK;
    size_t ssid_len = MAX_SSID_LEN;
    size_t pwd_len = MAX_PASSPHRASE_LEN;

    nvs_handle_t nvs_handle;
    ret = nvs_open("storage", NVS_READONLY, &nvs_handle);
    if (ret != ESP_OK) {
        ESP_LOGI(tag, "open nvs flash failed, return %s", esp_err_to_name(ret));
        return false;
    }

    if (nvs_get_str(nvs_handle, "ssid", ssid, &ssid_len) != ESP_OK ||
        nvs_get_str(nvs_handle, "password", password, &pwd_len) != ESP_OK) {
        nvs_close(nvs_handle);
        ESP_LOGE(tag, "read  WiFi credentials failed");
        return false;
    }

    nvs_close(nvs_handle);
    ESP_LOGI(tag, "read  WiFi credentials: SSID=%s, PASSWORD=%s", ssid, password);

    return true;
}

static void event_handler(void *arg, esp_event_base_t event_base,
                          int32_t event_id, void *event_data)
{
    WiFiConnect *wifi = static_cast<WiFiConnect *>(arg);

    if (event_base == WIFI_EVENT) {
        switch (event_id) {
        case WIFI_EVENT_STA_START:
            break;
        case WIFI_EVENT_STA_DISCONNECTED:
            esp_wifi_connect();
            xEventGroupClearBits(wifi->wifi_event_group_, WIFI_CONNECTED);
            break;
        default:
            break;
        }
    }

    if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        xEventGroupSetBits(wifi->wifi_event_group_, WIFI_CONNECTED);
        xEventGroupClearBits(wifi->wifi_event_group_, WIFI_CONNECTING);
    }

    if (event_base ==  SC_EVENT) {
        switch (event_id) {
        case SC_EVENT_SCAN_DONE:
            ESP_LOGI(tag, "Scan done");
            break;

        case SC_EVENT_FOUND_CHANNEL:
            ESP_LOGI(tag, "Found channel");
            break;

        case SC_EVENT_GOT_SSID_PSWD: {
            ESP_LOGI(tag, "got SSID and password");
            smartconfig_event_got_ssid_pswd_t *event = (smartconfig_event_got_ssid_pswd_t *)event_data;
            wifi_config_t wifi_config;
            char ssid[MAX_SSID_LEN] = {};
            char password[MAX_PASSPHRASE_LEN] = {};

            bzero(&wifi_config, sizeof(wifi_config));
            memcpy(wifi_config.sta.ssid, event->ssid, sizeof(wifi_config.sta.ssid));
            memcpy(wifi_config.sta.password, event->password, sizeof(wifi_config.sta.password));

            wifi_config.sta.bssid_set = event->bssid_set;
            if (wifi_config.sta.bssid_set) {
                ESP_LOGI(tag, "set mac address target of AP: " MACSTR, MAC2STR(event->bssid));
                memcpy(wifi_config.sta.bssid, event->bssid, sizeof(wifi_config.sta.bssid));
            }

            memcpy(ssid, event->ssid, sizeof(event->ssid));
            memcpy(password, event->password, sizeof(event->password));
            save_wifi_credentials(ssid, password);
            ESP_LOGI(tag, "SSID: %s", ssid);
            ESP_LOGI(tag, "password: %s", password);
            esp_wifi_disconnect();
            esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
            esp_wifi_connect();
            break;
        }

        case SC_EVENT_SEND_ACK_DONE:
            xEventGroupClearBits(wifi->wifi_event_group_, SMARTCONFIG_START);
            xEventGroupSetBits(wifi->wifi_event_group_, SMARTCONFIG_END);
            break;

        default:
            break;
        }

    }
}

static void smartconfig_task(void *param)
{
    EventBits_t bits;
    static bool startup = true;
    WiFiConnect *wifi = static_cast<WiFiConnect *>(param);

    while (1) {
        if (startup) {
            ESP_LOGI(tag, "wait 20S to connect wifi");
            bits = xEventGroupWaitBits(wifi->wifi_event_group_, WIFI_CONNECTED,
                                            pdFALSE, pdFALSE, pdMS_TO_TICKS(20000));
            if (!(bits & WIFI_CONNECTED)) {
                xEventGroupClearBits(wifi->wifi_event_group_,
                                     WIFI_CONNECTING | WIFI_CONNECTED);
                xEventGroupSetBits(wifi->wifi_event_group_, SMARTCONFIG_START);
                smartconfig_run = true;
            }
            startup = false;
        }

        if (smartconfig_run) {
            smartconfig_start_config_t config = SMARTCONFIG_START_CONFIG_DEFAULT();
            esp_smartconfig_set_type(SC_TYPE_ESPTOUCH);
            esp_event_handler_register(SC_EVENT, ESP_EVENT_ANY_ID, &event_handler, wifi);
            esp_smartconfig_start(&config);
            // 等待wifi连接
            bits = xEventGroupWaitBits(wifi->wifi_event_group_, WIFI_CONNECTED,
                                        true, false, portMAX_DELAY);
            if(bits & WIFI_CONNECTED) {
                xEventGroupSetBits(wifi->wifi_event_group_, WIFI_CONNECTED);
                ESP_LOGI(tag, "WiFi Connected to ap");
            }
            smartconfig_run = false;
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

esp_err_t WiFiConnect::init(void)
{
    char ssid[MAX_SSID_LEN] = {0};
    char password[MAX_PASSPHRASE_LEN] = {0};

    esp_netif_create_default_wifi_sta();
    wifi_init_config_t wifi_config = WIFI_INIT_CONFIG_DEFAULT();

    ESP_ERROR_CHECK(esp_netif_init());
    wifi_event_group_ = xEventGroupCreate();
    ESP_ERROR_CHECK(esp_wifi_init(&wifi_config));

    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, this);
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, this);
    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_start();
    esp_err_t ret = esp_wifi_set_inactive_time(WIFI_IF_STA, 30);
    if (ESP_OK != ret)
        ESP_LOGI(tag, "esp_wifi_set_inactive_time failed");

    if (read_wifi_credentials(ssid, password)) {
        wifi_config_t config;
        bzero(&config, sizeof(config));
        memcpy(config.sta.ssid, ssid, sizeof(config.sta.ssid));
        memcpy(config.sta.password, password, sizeof(config.sta.password));
        esp_wifi_set_config(WIFI_IF_STA, &config);
        esp_wifi_connect();
        xEventGroupSetBits(wifi_event_group_, WIFI_CONNECTING);
    }

    xTaskCreate(smartconfig_task, "smartconfig_task", 4096, this, 3, NULL);
    ESP_LOGI(tag, "start smartconfig and wait wifi connect");

    return ret ;
}
