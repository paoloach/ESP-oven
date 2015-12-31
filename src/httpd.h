#ifndef __HTTPD__H_
#define __HTTPD__H_
#include <esp8266.h>
#include <c_types.h>
#include <ip_addr.h>
#include "lwip/api.h"
#include "config.h"

#ifdef __cplusplus
extern "C" {
#endif

void initHttpd(void);

#ifdef __cplusplus
}
#endif

#endif
