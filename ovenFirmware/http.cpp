#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <esp_log.h>
#include "http.h"

constexpr const char *CONTENT_LENGTH = "Content-Length:";
constexpr const size_t CONTENT_LENGTH_SIZE = 15;
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


void HTTP::feed(const char *data) {
    if (status == HTTP_BODY) {
        *bodyIter = *data;
        bodyIter++;
        if ((bodyIter - body) == bodyLen) {
            ESP_LOGI(TAG,"END BODY and END PARSE");
            status = END_PARSE;
        }
    } else {
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
    const char *curData;
    const char *nextToken;
    ESP_LOGI(TAG, "line: %s", buffer);
    switch (status) {
        case FIRST_LINE: {
            curData = parseMethod(buffer);
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
                } else if (memcmp(curData, "HTTP/1.0", 8) == 0) {
                    httpVersion = HTTP_1_0;
                } else {
                    status = PARSE_ERROR;
                    ESP_LOGW(TAG, "unknown version: %s", curData);
                }
            } else {
                status = PARSE_ERROR;
                break;
            }
            status = HTTP_HEADERS;
        }
            break;
        case HTTP_HEADERS:
            if (bufferIter - buffer >= 1) {
                if (buffer[0] == '\r' ) {
                    if (bodyLen > 0) {
                        ESP_LOGI(TAG,"HTTP BODY start");
                        status = HTTP_BODY;
                    } else {
                        status = END_PARSE;
                    }
                } else {
                    parseHeader();
                }
            } else {
                if (bodyLen > 0) {
                    status = HTTP_BODY;
                } else {
                    status = END_PARSE;
                }
            }
            break;
        case END_PARSE:
            print();
            break;
        case PARSE_ERROR:
            ESP_LOGE(TAG, "Parse error");
            break;
    }
}


void HTTP::parseHeader() {
    size_t bufferLen = bufferIter - buffer;
    if (bufferLen >= CONTENT_LENGTH_SIZE && memcmp(buffer, CONTENT_LENGTH,CONTENT_LENGTH_SIZE)==0) {
        ESP_LOGI(TAG, "Content length: %s", &buffer[CONTENT_LENGTH_SIZE + 1]);
        bodyLen = atoi(&buffer[CONTENT_LENGTH_SIZE + 1]);
        body = new char[bodyLen + 1];
        body[bodyLen] = 0;
        bodyIter = body;
        ESP_LOGI(TAG, "Body len: %d", bodyLen);
    }
}

void HTTP::print() {
    ESP_LOGD(TAG, "method: %d", method);
    ESP_LOGD(TAG, "url: %s", url);
    ESP_LOGD(TAG, "headers found: %d", headers.size());
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
