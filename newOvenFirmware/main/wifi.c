//
// Created by paolo on 31/05/19.
//

#include <esp_event_loop.h>
#include <esp_log.h>
#include <esp_wifi.h>
#include <nvs_flash.h>
#include <esp_http_server.h>

#include "httpServer.h"
#include "wifipasswd.h"

#include "wifi.h"
#include "GroupSignals.h"

static const char *TAG = "Wifi";
EventGroupHandle_t wifi_event_group;

static esp_err_t event_handler(void *ctx, system_event_t *event) {
    httpd_handle_t *server = (httpd_handle_t *) ctx;

    switch (event->event_id) {
        case SYSTEM_EVENT_STA_START:
            ESP_LOGI(TAG, "SYSTEM_EVENT_STA_START");
            esp_err_t ret = tcpip_adapter_set_hostname(TCPIP_ADAPTER_IF_STA ,"Vase-2.0");
            if(ret != ESP_OK ){
                ESP_LOGE(TAG,"failed to set hostname:%X",ret);
            }
            ESP_ERROR_CHECK(esp_wifi_connect());
            break;
        case SYSTEM_EVENT_STA_GOT_IP:
            ESP_LOGI(TAG, "SYSTEM_EVENT_STA_GOT_IP");
            ESP_LOGI(TAG, "Got IP: '%s'", ip4addr_ntoa(&event->event_info.got_ip.ip_info.ip));

            /* Start the web server */
            if (*server == NULL) {
                *server = start_webserver();
            }
            xEventGroupSetBits(wifi_event_group, VASO_CONNECTED_BIT);
            break;
        case SYSTEM_EVENT_STA_DISCONNECTED:
            ESP_LOGI(TAG, "SYSTEM_EVENT_STA_DISCONNECTED");
            ESP_ERROR_CHECK(esp_wifi_connect());

            /* Stop the web server */
            if (*server) {
                stop_webserver();
                *server = NULL;
            }
            break;
        default:
            ESP_LOGI(TAG, "event: %d", event->event_id);
            break;
    }
    return ESP_OK;
}

void initWifi() {
    wifi_event_group = xEventGroupCreate();
    tcpip_adapter_init();



    ESP_ERROR_CHECK(esp_event_loop_init(event_handler, &server));
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));

    wifi_config_t wifi_config = {
            .sta = {
                    .ssid = WIFI_SSID,
                    .password = WIFI_PASS
            },
    };

    ESP_LOGI(TAG, "Setting WiFi configuration SSID %s...", wifi_config.sta.ssid);
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());


}
