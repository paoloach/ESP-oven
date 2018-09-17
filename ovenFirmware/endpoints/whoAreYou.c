//
// Created by paolo on 12/09/18.
//
#include "whoAreYou.h"
#include "../httpd/include/httpd.h"

int cgiWhoAreYou(HttpdConnData *connData){
    if (connData->conn==NULL) {
        //Connection aborted. Clean up.
        return HTTPD_CGI_DONE;
    }
    if (connData->requestType == HTTPD_METHOD_GET) {
        httpdStartResponse(connData, 200);
        httpdHeader(connData, "Content-Type", "text/plain");
        httpdHeader(connData, "Content-Length", "14");
        httpdEndHeaders(connData);
        httpdSend(connData, RESPONSE, -1);
        return HTTPD_CGI_DONE;
    } else {
        httpdStartResponse(connData, 404);
        return HTTPD_CGI_DONE;
    }
}
