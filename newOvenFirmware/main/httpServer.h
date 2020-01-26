//
// Created by paolo on 30/05/19.
//

#ifndef VAS2_ESP8266_HTTPSERVER_H
#define VAS2_ESP8266_HTTPSERVER_H

#include <esp_http_server.h>
httpd_handle_t start_webserver(void);
void stop_webserver(void);
extern httpd_handle_t server;

#endif //VAS2_ESP8266_HTTPSERVER_H
