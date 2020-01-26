//
// Created by paolo on 30/05/19.
//
#include <ctype.h>
#include <esp_log.h>
#include <nvs_flash.h>

#include <esp_http_server.h>
#include <freertos/event_groups.h>

static const char *TAG = "HttpServer";
httpd_handle_t server = NULL;

extern int degree;
extern int threshold;
extern bool enable;
extern bool restart;

static esp_err_t methodNotSupported(httpd_req_t *connData);

esp_err_t aboutHandler(httpd_req_t *req) {
    httpd_resp_set_type(req, HTTPD_TYPE_TEXT);
    httpd_resp_send(req, "I am ESP-OVEN\n", -1);

    return ESP_OK;
}

esp_err_t getTemperatureHandler(httpd_req_t * req){
    char buffer[10];
    sprintf(buffer, "%d\n", degree);

    httpd_resp_set_type(req, HTTPD_TYPE_TEXT);
    httpd_resp_send(req, buffer, -1);

    return ESP_OK;
}

esp_err_t getThresholdHandler(httpd_req_t * req){
    char buffer[10];
    sprintf(buffer, "%d\n", threshold);

    httpd_resp_set_type(req, HTTPD_TYPE_TEXT);
    httpd_resp_send(req, buffer, -1);

    return ESP_OK;
}

esp_err_t setThresholdHandler(httpd_req_t * req){
    char body[20];
    int bodyLength = httpd_req_recv(req, body, sizeof(body));

    const char *end = body + bodyLength;
    const char * iter = body;
    for (; iter < end; iter++) {
        int c = *iter;
        if (!isdigit(c)) {
            ESP_LOGI("HTTPD","ERROR: body is not a number\n");
            httpd_resp_set_status(req, "400 (Bad Request)");
            return ESP_OK;
        }
    }
    threshold = atoi(body);
    ESP_LOGI(TAG, "new threshold: %d\n", threshold);

    char buffer[10];
    sprintf(buffer, "%d\n", threshold);

    httpd_resp_send(req, buffer, -1);
    return ESP_OK;
}


httpd_handle_t start_webserver(void) {
    httpd_handle_t httpServer = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.max_uri_handlers = 15;

    httpd_uri_t getWhoAreYou = {.uri = "/whoareyou", .method = HTTP_GET, .handler = aboutHandler, .user_ctx = NULL};
    httpd_uri_t getAbout = {.uri = "/about", .method = HTTP_GET, .handler = aboutHandler, .user_ctx = NULL};
    httpd_uri_t getTemperature = {.uri = "/temperature", .method = HTTP_GET, .handler = getTemperatureHandler, .user_ctx = NULL};
    httpd_uri_t getThreshold = {.uri = "/threshold", .method = HTTP_GET, .handler = getThresholdHandler, .user_ctx = NULL};
    httpd_uri_t setTemperature = {.uri = "/threshold", .method = HTTP_PUT, .handler = setThresholdHandler, .user_ctx = NULL};

    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&httpServer, &config) == ESP_OK) {
        ESP_LOGI(TAG, "Registering URI handlers");
        httpd_register_uri_handler(httpServer, &getWhoAreYou);
        httpd_register_uri_handler(httpServer, &getAbout);
        httpd_register_uri_handler(httpServer, &getTemperature);
        httpd_register_uri_handler(httpServer, &getThreshold);
        httpd_register_uri_handler(httpServer, &setTemperature);
        ESP_LOGI(TAG, "HTTP SERVER STARTED");
        return httpServer;
    }

    ESP_LOGI(TAG, "Error starting server!");
    return NULL;
}

void stop_webserver(void) {
    ESP_LOGI(TAG, "Web server stop");
    httpd_stop(server);
}


