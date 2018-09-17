#ifndef __HTTP_RESPONSE__H__
#define  __HTTP_RESPONSE__H__

class HttpResponse {
public:
    static bool sendOk(int fd, const char *response);

    static void sendNotFound(int fd);

    static void sendBadRequest(int fd);
};


#endif
