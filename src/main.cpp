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
#include "pwm.h"
#include "PID.h"
#include "ErrorHistory.h"

int degree;
int threshold=110;
bool enable=true;
bool restart=false;

static void on_off(int on_perc);
static void warnup(ErrorHistory &errorHistory, PID & pid);
static void readTemp();

extern "C"{
  void user_init(void);
}

void gpio_output_set(uint32 set_mask, uint32 clear_mask, uint32 enable_mask, uint32 disable_mask)
{
    GPIO_REG_WRITE(GPIO_OUT_W1TS_ADDRESS, set_mask);
    GPIO_REG_WRITE(GPIO_OUT_W1TC_ADDRESS, clear_mask);
    GPIO_REG_WRITE(GPIO_ENABLE_W1TS_ADDRESS, enable_mask);
    GPIO_REG_WRITE(GPIO_ENABLE_W1TC_ADDRESS, disable_mask);
}

void readTempNoPrint(){
  SET_PERI_REG_MASK(SPI_CMD(HSPI), SPI_USR);
  int temp = READ_PERI_REG(SPI_W0(HSPI)) >> 16;
  degree = temp /32;
}

void readTemp(){
  readTempNoPrint();
  printf("temp: %d\n",degree );
}

void spi_task(void *pvParameters){
  printf("SPI TASK START\n");
  printf("portTICK_RATE_MS: %d\n", portTICK_RATE_MS);

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
    if (errorHistory.getSize() > 20 && errorHistory.maxMeanError(tempThreshold) < 5){
      return;
    }
  }
}

static void on_off(int on_perc) {
  int onPeriod=on_perc*20;
  int offPeriod= (100-on_perc)*20;
  if (onPeriod>0){
    GPIO_OUTPUT_SET(GPIO_ID_PIN(2), 1);
    vTaskDelay(onPeriod/ portTICK_RATE_MS);
  }
  if (offPeriod>0){
    GPIO_OUTPUT_SET(GPIO_ID_PIN(2), 0);
    vTaskDelay(offPeriod/ portTICK_RATE_MS);
  }
}

void user_init(void){
    uart_set_baud(0, 115200);
    printf("SDK version:%s\n", sdk_system_get_sdk_version());

//    struct sdk_station_config config = {
//        .ssid = WIFI_SSID,
//        .password = WIFI_PASS,
//    };

    struct sdk_station_config config {WIFI_SSID, WIFI_PASS};
    sdk_wifi_set_opmode(STATION_MODE);
    sdk_wifi_station_set_config(&config);

    xTaskCreate(&spi_task, (signed char *)"SPI_task", 256, NULL, 2, NULL);
    initHttpd();
}
