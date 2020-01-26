//
// Created by paolo on 12/09/18.
//

#include <esp8266/spi_register.h>
#include <esp_log.h>
#include "spi.h"
#include "gpio.h"
#include "errorHistory.h"
#include "PID.h"

int degree;
int threshold = 110;
bool enable = true;
bool restart = false;

const char * TAG="SPI";

static void readTempNoPrint() {
    int temp = spi_rx16(HSPI);
//    SET_PERI_REG_MASK(SPI_CMD(HSPI), SPI_USR);
//    int temp = READ_PERI_REG(SPI_W0(HSPI)) >> 16;
    degree = temp / 32;
}

static void readTemp() {

    readTempNoPrint();
    ESP_LOGI(TAG,"Read temp %d", degree);
}

static void on_off(int on_perc) {
    int onPeriod = on_perc * 20;
    int offPeriod = (100 - on_perc) * 20;
    if (onPeriod > 0) {
        gpio_set_level(GPIO_NUM_2, 1);
        vTaskDelay(onPeriod / portTICK_PERIOD_MS);
    }
    if (offPeriod > 0) {
        gpio_set_level(GPIO_NUM_2, 0);
        vTaskDelay(offPeriod / portTICK_PERIOD_MS);
    }
}


static void warnup(ErrorHistory &errorHistory, PID &pid) {
    int on_perc;

    while (1) {
        // tempThreshold is not a costant for the cycle because can be varied by a rest command
        int tempThreshold = threshold - 10;
        readTemp();
        int err = tempThreshold - degree;
        errorHistory.addError(err);
        on_perc = pid.calc(err);
        if (!enable) {
            on_perc = 0;
        }
        on_off(on_perc);
        if (errorHistory.getSize() > 20 && errorHistory.maxMeanError() < 5) {
            return;
        }
    }
}


#ifdef __cplusplus
extern "C" {
#endif
void spi_task(void *pvParameters) {
    ESP_LOGI(TAG, "SPI_TASK");
    spi_init_gpio(HSPI, SPI_CLK_USE_DIV);
    ESP_LOGI(TAG, "A");
    spi_clock(HSPI, 8, 10);
    ESP_LOGI(TAG, "B");
    spi_rx_byte_order(HSPI, SPI_BYTE_ORDER_HIGH_TO_LOW);
    ESP_LOGI(TAG, "C");
//    CLEAR_PERI_REG_MASK(SPI_USER(HSPI), SPI_USR_MOSI | SPI_USR_MISO | SPI_USR_COMMAND | SPI_USR_ADDR | SPI_USR_DUMMY);
//    ESP_LOGI(TAG, "C1");
//    SET_PERI_REG_MASK(SPI_USER(HSPI), SPI_USR_MISO);
//    ESP_LOGI(TAG, "C2");
//    WRITE_PERI_REG(SPI_USER1(HSPI), (15 & SPI_USR_MISO_BITLEN) << SPI_USR_MISO_BITLEN_S);
    ESP_LOGI(TAG, "D");
    gpio_config_t gpioConfig;
    gpioConfig.intr_type=GPIO_INTR_DISABLE;
    gpioConfig.mode=GPIO_MODE_OUTPUT;
    gpioConfig.pin_bit_mask=GPIO_Pin_2;
    gpioConfig.pull_down_en=GPIO_PULLDOWN_ENABLE;
    gpioConfig.pull_up_en=GPIO_PULLUP_DISABLE;
    gpio_config(&gpioConfig);
    gpio_set_level(GPIO_NUM_2, 1);
    ESP_LOGI(TAG, "E");
    ErrorHistory errorHistory;
    PID pid(errorHistory);

    ESP_LOGI(TAG, "START SPI");

    while (true) {
        restart = false;
        warnup(errorHistory, pid);
        int on_perc;

        while (!restart) {
            readTemp();
            int err = threshold - degree;
            errorHistory.addError(err);
            on_perc = pid.calc(err);
            if (!enable) {
                on_perc = 0;
            }
            on_off(on_perc);
        }
    }
}
#ifdef __cplusplus
};
#endif