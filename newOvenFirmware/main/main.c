#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_wifi.h"
#include "esp_log.h"

#include <spi_flash.h>
#include "temperature.h"
#include "wifi.h"

static const char *TAG = "OVEN";

void app_main() {
    ESP_LOGI(TAG,"Starting!");
    uint8_t mac[8];

    /* Print chip information */
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    ESP_LOGI(TAG,"This is ESP8266 chip with %d CPU cores, WiFi%s%s, ",
           chip_info.cores,
           (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
           (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");

    ESP_LOGI(TAG,"silicon revision %d, ", chip_info.revision);

    ESP_LOGI(TAG,"%dMB %s flash\n", spi_flash_get_chip_size() / (1024 * 1024),
           (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");

    esp_efuse_mac_get_default(mac);
    ESP_LOGI(TAG, "Default mac: "MACSTR, MAC2STR(mac));

    initWifi();

    xTaskCreate(spi_task, "SPI_task", 2048, NULL, 2, NULL);
}