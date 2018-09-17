#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <esp_log.h>
#include "http.h"


constexpr const char *TAG = "http";
constexpr size_t BUFFER_LEN = 100;
static char buffer[BUFFER_LEN];
static char previousChar;
constexpr char *bufferEnd = buffer + BUFFER_LEN;

HTTP::HTTP() {
    bufferIter = buffer;
    status = HTTP::FIRST_LINE;
    httpVersion = HTTP_UNKNOWN_VERSION;
    url = NULL;
    body = NULL;
    bodyLen = 0;
    headers.clear();
}

HTTP::~HTTP() {
    delete body;
    delete url;
}


void HTTP::feed(const char *data, u16_t len) {
    while (len > 0) {
        if (previousChar == '\r' && *data == '\n') {
            *bufferIter = 0;
            feedNewLine();
            bufferIter = buffer;
            previousChar = 0;
        } else {
            previousChar = *data;
            if (bufferIter < bufferEnd) {
                *bufferIter = *data;
                bufferIter++;
            }
        }
    }
}

void HTTP::feedNewLine() {
    ESP_LOGI(TAG, "new line: %s", buffer);
    const char *curData = buffer;
    const char *nextToken;
    while (curData < bufferIter) {
        switch (status) {
            case FIRST_LINE: {
                curData = parseMethod(curData);
                curData = removeSpaces(curData);
                nextToken = findSpace(curData);
                if (nextToken >= bufferIter) {
                    status = PARSE_ERROR;
                    break;
                }
                int size = nextToken - curData;
                url = new char[size + 1];
                memcpy(url, curData, size);
                url[size] = 0;
                curData = removeSpaces(nextToken);
                if (bufferIter - curData >= 8) {
                    if (memcmp(curData, "HTTP/1.1", 8) == 0) {
                        httpVersion = HTTP_1_1;
                        ESP_LOGD(TAG, "HTTP 1.1");
                    } else if (memcmp(curData, "HTTP/1.0", 8) == 0) {
                        httpVersion = HTTP_1_0;
                        ESP_LOGI(TAG, "HTTP 1.0");
                    } else {
                        status = PARSE_ERROR;
                        ESP_LOGW(TAG, "unknown version: %s", curData);
                    }
                } else {
                    status = PARSE_ERROR;
                    break;
                }
                status = HTTP_HEADERS;
                break;
            }
            case HTTP_HEADERS:
                if (bufferIter - curData >= 2) {
                    if (curData[0] == '\r' && curData[1] == '\n') {
                        status = HTTP_BODY;
                    }
                } else {
                    status = END_PARSE;
                }
                break;
            case HTTP_BODY:
                bodyLen = bufferIter - curData;
                if (bodyLen > 0) {
                    body = new char[bodyLen + 1];
                    memcpy(body, curData, bodyLen);
                    body[bodyLen] = 0;
                }
                status = END_PARSE;
                break;
            case END_PARSE:
                print();
                break;
            case PARSE_ERROR:
                ESP_LOGE(TAG, "Parse error");
                break;
        }
    }
}

void HTTP::print() {
    ESP_LOGD(TAG,"method: %d", method);
    ESP_LOGD(TAG,"url: %s", url);
    ESP_LOGD(TAG,"headers found: %d", headers.size());
    if (body != NULL) {
        ESP_LOGD(TAG, "body: %s", body);
    }

    int index = 0;
    std::vector<HttpHeader>::iterator iter;
    for (iter = headers.begin(); iter != headers.end(); iter++) {
        HttpHeader &header = *iter;
        ESP_LOGD(TAG, "%d)  ", index);
        header.print();
        ESP_LOGD(TAG, "\n");
        index++;
    }
}

const char *HTTP::removeSpaces(const char *data) {
    while (isspace(*data) && data < bufferIter) {
        data++;
    }
    return data;
}

const char *HTTP::findSpace(const char *data) {
    while (!isspace(*data) && data < bufferIter) {
        data++;
    }
    return data;
}

const char *HTTP::parseMethod(const char *data) {
    size_t len = bufferIter - data;

    if (len >= 7 && memcmp(data, "OPTIONS", 7) == 0) {
        data += 7;
        method = OPTIONS;
    } else if (len >= 4 && memcmp(data, "GET", 3) == 0) {
        data += 4;
        method = GET;
    } else if (len >= 5 && memcmp(data, "HEAD", 4) == 0) {
        data += 5;
        method = HEAD;
    } else if (len >= 5 && memcmp(data, "POST", 4) == 0) {
        data += 5;
        method = POST;
    } else if (len >= 4 && memcmp(data, "PUT", 3) == 0) {
        data += 4;
        method = PUT;
    } else if (len >= 7 && memcmp(data, "DELETE", 6) == 0) {
        data += 7;
        method = DELETE;
    } else if (len >= 6 && memcmp(data, "TRACE", 5) == 0) {
        data += 6;
        method = TRACE;
    } else if (len >= 7 && memcmp(data, "CONNECT", 7) == 0) {
        data += 7;
        method = CONNECT;
    } else {
        method = UNDEFINED_METHOD;
    }

    return data;
}
