#include <vector>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <lwip/sockets.h>
#include <esp_log.h>
#include "freertos/FreeRTOS.h"
#include <freertos/event_groups.h>

#include "lwip/err.h"
#include "lwip/sys.h"
#include "lwip/mem.h"
#include "lwip/memp.h"
#include "lwip/pbuf.h"
#include "lwip/tcp.h"

#include "netif/etharp.h"

#include "httpd.h"
#include "ovenConfig.h"
#include "http.h"
#include "UrlMatcher.h"
#include "httpResponse.h"


extern int degree;
extern int threshold;
extern bool enable;
extern bool restart;

//static void getRoot(Connection * c);
static void getWhoAreYou(int fd, HTTP &http);

static void getTemperature(int fd, HTTP &http);

static void getThreshold(int fd, HTTP &http);

static void setTemperature(int fd, HTTP &http);

static void setEnable(int fd, HTTP &http);

static void setRestart(int fd, HTTP &http);

static bool readRequest(int fd, HTTP &http);

static void dispatch(int newConnection, HTTP &http);

const constexpr char * TAG = "httpd";
std::vector<UrlMatcher> matchersGet;
std::vector<UrlMatcher> matchersPut;
std::vector<UrlMatcher> matchersEmpty;

extern "C" void sdk_system_print_meminfo(void);
extern "C" EventGroupHandle_t wifi_event_group;
const int CONNECTED_BIT = 1 >> 0;

inline bool isLastPBuf(const struct pbuf *p) {
    return p->len == p->tot_len;
}


void whoAreYouFunc(int fd, HTTP &http) {
    HttpResponse::sendOk(fd, "I am ESP-OVEN");
}


void httpTask(void *) {
    err_t error;
    int serverSocket;

    ESP_LOGI("http", "Start httpd");
    matchersGet.reserve(4);
    matchersGet.emplace_back("/", whoAreYouFunc);
    matchersGet.emplace_back("/whoareyou", getWhoAreYou);
    matchersGet.emplace_back("/temperature", getTemperature);
    matchersGet.emplace_back("/threshold", getThreshold);

    matchersPut.emplace_back("/threshold", setTemperature);
    matchersPut.emplace_back("/enable", setEnable);
    matchersPut.emplace_back("/restart", setRestart);

    xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT,
                        false, true, portMAX_DELAY);
    ESP_LOGI(TAG, "Connected to AP");

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        ESP_LOGE("httpd", "Unable to create a socket");
        return;
    }
    ESP_LOGI("http", "Socket created");
    int enable = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));

    struct sockaddr_in servAddr;
    bzero(&servAddr, sizeof(servAddr));
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY); /* Any incoming interface */
    servAddr.sin_port = htons(HTTP_PORT);
    servAddr.sin_len = sizeof(servAddr);
    servAddr.sin_family = AF_INET;
    error = bind(serverSocket, (struct sockaddr *) &servAddr, sizeof(servAddr));
    if (error != ERR_OK) {
        ESP_LOGE("httpd", "bind error: %d\n", error);
        return;
    }

    error = listen(serverSocket, 5);
    if (error != ERR_OK) {
        ESP_LOGE("httpd", "listen error: %d\n", error);
        return;
    }
    while (true) {
        struct sockaddr_in connection;
        socklen_t socklen = sizeof(struct sockaddr_in);
        int newConnection = accept(serverSocket, (struct sockaddr *) &connection, &socklen);
        if (newConnection >= 0) {
            HTTP http;
            if (readRequest(newConnection, http)) {
                dispatch(newConnection, http);
            }
            close(newConnection);
        }
    }
}

bool readRequest(int fd, HTTP &http) {
    fd_set read_fds;
    struct timeval tv;
    int selectRes;
    char data[4];
    fcntl(fd, F_SETFL, O_NONBLOCK);
    while (true) {
        FD_ZERO(&read_fds);
        FD_SET(fd, &read_fds);
        tv.tv_sec = 10;
        tv.tv_usec = 0;
        selectRes = select(fd + 1, &read_fds, NULL, NULL, &tv);
        if (selectRes > 0) {
            if (read(fd, data, 1) > 0)
                http.feed(data);
        } else if (selectRes < 0) {
            return false;
        }
        if (http.error()) {
            ESP_LOGE(TAG, "Parse error (%s:%d)",__FILE__,__LINE__);
            return false;
        }
        if (!http.needMoreData()) {
            return true;
        }
    }
}

void dispatch(int fd, HTTP &http) {
    std::vector<UrlMatcher> *matchers;
    switch (http.getMethod()) {
        case GET:
            matchers = &matchersGet;
            break;
        case PUT:
            matchers = &matchersPut;
            break;
        default:
            matchers = &matchersEmpty;
    }
    bool found = false;
    std::vector<UrlMatcher>::iterator iter;
    for (UrlMatcher matcher: *matchers) {
        if (matcher == http.getUrl()) {
            found = true;
            matcher(fd, http);
        }
    }
    if (!found) {
        ESP_LOGW(TAG,"Matcher NOT found");
        HttpResponse::sendNotFound(fd);
    }

}

void getWhoAreYou(int fd, HTTP &http) {
    ESP_LOGI("httpd", "Respond at whoareyou");
    HttpResponse::sendOk(fd, "I am ESP-OVEN");
}

void getTemperature(int fd, HTTP &http) {
    char buffer[10];
    sprintf(buffer, "%d", degree);
    printf("Respond at temperature\n");
    HttpResponse::sendOk(fd, buffer);
}

void getThreshold(int fd, HTTP &http) {
    char buffer[10];
    sprintf(buffer, "%d", threshold);
    printf("Respond at threshold\n");
    HttpResponse::sendOk(fd, buffer);
}

void setTemperature(int fd, HTTP &http) {
    ESP_LOGI("HTTP", "Set Threashold");
    const char *body = http.getBody();
    const char *iter = body;
    const char *end = body + http.getBodyLength();
    for (; iter < end; iter++) {
        if (!isdigit(*iter)) {
            ESP_LOGI("HTTPD","ERROR: body is not a number\n");
            HttpResponse::sendBadRequest(fd);
            return;
        }
    }
    threshold = atoi(body);
    ESP_LOGI(TAG, "new threshold: %d\n", threshold);
    HttpResponse::sendOk(fd, nullptr);
}

void setEnable(int fd, HTTP &http) {
    const char *body = http.getBody();
    ESP_LOGI(TAG,"Body: %s\n", body);
    if (strcmp(body, "1")) {
        enable = true;
    }
    if (strcmp(body, "true")) {
        enable = true;
    }
    if (strcmp(body, "0")) {
        enable = false;
    }
    if (strcmp(body, "false")) {
        enable = false;
    }

    ESP_LOGI(TAG, "enable: %d\n", enable);
    HttpResponse::sendOk(fd, nullptr);
}

void setRestart(int fd, HTTP &http) {
    const char *body = http.getBody();
    ESP_LOGD(TAG, "Body: %s\n", body);
    if (strcmp(body, "1")) {
        restart = true;
    }
    if (strcmp(body, "true")) {
        restart = true;
    }
    if (strcmp(body, "0")) {
        restart = false;
    }
    if (strcmp(body, "false")) {
        restart = false;
    }

    ESP_LOGD(TAG, "restart: %d\n", restart);
    HttpResponse::sendOk(fd, nullptr);
}
