#include <lwip/sockets.h>
#include <string.h>
#include "httpResponse.h"


constexpr const char statusLineOK[] = "HTTP/1.1 200 OK\r\n";
constexpr const char statusLineBadRequest[] = "HTTP/1.1 400 Bad request\r\n";
constexpr const char statusLineNotFound[] = "HTTP/1.1 404 Not Found\r\n";
constexpr const char contentLengthHeader[] = "Content-Length:";


void HttpResponse::sendNotFound(int fd) {
    send(fd, statusLineNotFound, sizeof(statusLineNotFound) - 1, MSG_MORE);
    send(fd, contentLengthHeader, sizeof(contentLengthHeader) - 1, MSG_MORE);
    send(fd, "0\r\n\r\n", 5, 0);
}

void HttpResponse::sendBadRequest(int fd) {
    send(fd, statusLineBadRequest, sizeof(statusLineBadRequest) - 1, MSG_MORE);
    send(fd, contentLengthHeader, sizeof(contentLengthHeader) - 1, MSG_MORE);
    send(fd, "0\r\n\r\n", 5, 0);
}

bool HttpResponse::sendOk(int fd, const char *response) {
    char buffer[10];
    int dataLen = 0;
    if (response != nullptr) {
        dataLen = strlen(response);
    }
    send(fd, statusLineOK, sizeof(statusLineOK) - 1, MSG_MORE);
    send(fd, contentLengthHeader, sizeof(contentLengthHeader) - 1, MSG_MORE);
    auto bufferLen = (size_t)sprintf(buffer, "%d\r\n", dataLen);
    send(fd, buffer, bufferLen, MSG_MORE);
    send(fd, "\r\n", 2, 0);
    if (dataLen > 0) {
        send(fd, response, dataLen, 0);
    }
    return true;
}
