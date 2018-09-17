//
// Created by paolo on 12/09/18.
//

#include <c_types.h>
#include "platform.h"
#include "getTemperature.h"
#include "../httpd/include/httpd.h"

int getTemperature(HttpdConnData *connData) {
    if (connData->conn == NULL) {
        //Connection aborted. Clean up.
        return HTTPD_CGI_DONE;
    }
    if (connData->requestType == HTTPD_METHOD_GET) {

    }
}