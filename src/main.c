#include "espressif/esp_common.h"
#include "esp/uart.h"

#include <string.h>
#include <unistd.h>

#include "FreeRTOS.h"
#include "task.h"


#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"

#include "ssid_config.h"
#include "httpd.h"
#include "spi.h"
#include "gpio.h"

int degree;
int threshold=40;

void
gpio_output_set(uint32 set_mask, uint32 clear_mask, uint32 enable_mask, uint32 disable_mask)
{
    GPIO_REG_WRITE(GPIO_OUT_W1TS_ADDRESS, set_mask);
    GPIO_REG_WRITE(GPIO_OUT_W1TC_ADDRESS, clear_mask);
    GPIO_REG_WRITE(GPIO_ENABLE_W1TS_ADDRESS, enable_mask);
    GPIO_REG_WRITE(GPIO_ENABLE_W1TC_ADDRESS, disable_mask);
}

void spi_task(void *pvParameters){
  const portTickType xDelay = 500 / portTICK_RATE_MS;
  printf("SPI TASK START\n");
  printf("xDelay: %d\n", xDelay);


  spi_init_gpio(HSPI,SPI_CLK_USE_DIV);
  spi_clock(HSPI,8,10 );
  spi_rx_byte_order(HSPI,SPI_BYTE_ORDER_HIGH_TO_LOW);
  CLEAR_PERI_REG_MASK(SPI_USER(HSPI), SPI_USR_MOSI|SPI_USR_MISO|SPI_USR_COMMAND|SPI_USR_ADDR|SPI_USR_DUMMY);
  SET_PERI_REG_MASK(SPI_USER(HSPI), SPI_USR_MISO);
  WRITE_PERI_REG(SPI_USER1(HSPI), (15&SPI_USR_MISO_BITLEN)<<SPI_USR_MISO_BITLEN_S);

  PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U, FUNC_GPIO2);
  GPIO_OUTPUT_SET(GPIO_ID_PIN(2), 1);

  while(1) {
    SET_PERI_REG_MASK(SPI_CMD(HSPI), SPI_USR);
    int temp = READ_PERI_REG(SPI_W0(HSPI)) >> 16;
    degree = temp /32;
    if (degree < threshold){
      GPIO_OUTPUT_SET(GPIO_ID_PIN(2), 1);
    } else {
      GPIO_OUTPUT_SET(GPIO_ID_PIN(2), 0);
    }
  //  printf("temp %d , threshold: %d\n", degree, threshold);
    vTaskDelay(xDelay);
  }

}

void user_init(void)
{
    uart_set_baud(0, 115200);
    printf("SDK version:%s\n", sdk_system_get_sdk_version());

    struct sdk_station_config config = {
        .ssid = WIFI_SSID,
        .password = WIFI_PASS,
    };
    /* required to call wifi_set_opmode before station_set_config */
    sdk_wifi_set_opmode(STATION_MODE);
    sdk_wifi_station_set_config(&config);

    xTaskCreate(&spi_task, (signed char *)"SPI_task", 256, NULL, 2, NULL);
    initHttpd();
}
