//
// Created by paolo on 12/09/18.
//
#include <esp_log.h>
#include "spi.h"
#include "gpio.h"
#include "errorHistory.h"
#include "PID.h"

int degree;
int threshold=110;
bool enable=true;
bool restart=false;


static void gpio_output_set(uint32_t  set_mask, uint32_t  clear_mask, uint32_t  enable_mask, uint32_t  disable_mask)
{
    GPIO_REG_WRITE(GPIO_OUT_W1TS_ADDRESS, set_mask);
    GPIO_REG_WRITE(GPIO_OUT_W1TC_ADDRESS, clear_mask);
    GPIO_REG_WRITE(GPIO_ENABLE_W1TS_ADDRESS, enable_mask);
    GPIO_REG_WRITE(GPIO_ENABLE_W1TC_ADDRESS, disable_mask);
}

static void readTempNoPrint(){
    SET_PERI_REG_MASK(SPI_CMD(HSPI), SPI_USR);
    int temp = READ_PERI_REG(SPI_W0(HSPI)) >> 16;
    degree = temp /32;
}

static void readTemp(){
    readTempNoPrint();
}

static void on_off(int on_perc) {
    int onPeriod=on_perc*20;
    int offPeriod= (100-on_perc)*20;
    if (onPeriod>0){
        GPIO_OUTPUT_SET(GPIO_ID_PIN(2), 1);
        vTaskDelay(onPeriod/ portTICK_PERIOD_MS);
    }
    if (offPeriod>0){
        GPIO_OUTPUT_SET(GPIO_ID_PIN(2), 0);
        vTaskDelay(offPeriod/ portTICK_PERIOD_MS);
    }
}

 

static void warnup(ErrorHistory &errorHistory, PID & pid) {
    int on_perc;

    while(1){
        // tempThreshold is not a costant for the cycle because can be varied by a rest command
        int tempThreshold = threshold-10;
        readTemp();
        int err = tempThreshold-degree;
        errorHistory.addError(err);
        on_perc = pid.calc(err);
        if (!enable){
            on_perc=0;
        }
        on_off(on_perc);
        if (errorHistory.getSize() > 20 && errorHistory.maxMeanError() < 5){
            return;
        }
    }
}


#ifdef __cplusplus
extern "C" {
#endif
void spi_task(void *pvParameters){
    spi_init_gpio(HSPI,SPI_CLK_USE_DIV);
    spi_clock(HSPI,8,10 );
    spi_rx_byte_order(HSPI,SPI_BYTE_ORDER_HIGH_TO_LOW);
    CLEAR_PERI_REG_MASK(SPI_USER(HSPI), SPI_USR_MOSI|SPI_USR_MISO|SPI_USR_COMMAND|SPI_USR_ADDR|SPI_USR_DUMMY);
    SET_PERI_REG_MASK(SPI_USER(HSPI), SPI_USR_MISO);
    WRITE_PERI_REG(SPI_USER1(HSPI), (15&SPI_USR_MISO_BITLEN)<<SPI_USR_MISO_BITLEN_S);

    PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U, FUNC_GPIO2);
    GPIO_OUTPUT_SET(GPIO_ID_PIN(2), 1);

    ErrorHistory errorHistory;
    PID pid(errorHistory);

    ESP_LOGI("SPI", "START SPI");

    while(true){
        restart=false;
        warnup(errorHistory, pid);
        int on_perc;

        while(!restart){
            readTemp();
            int err = threshold-degree;
            errorHistory.addError(err);
            on_perc = pid.calc(err);
            if (!enable){
                on_perc=0;
            }
            on_off(on_perc);
        }
    }
}
#ifdef __cplusplus
};
#endif