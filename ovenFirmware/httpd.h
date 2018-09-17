#ifndef __HTTPD__H_
#define __HTTPD__H_
#include "lwip/api.h"
#include "ovenConfig.h"

#ifdef __cplusplus
extern "C" {
#endif

void httpTask(void *);
void initHttpd(void);

#ifdef __cplusplus
}
#endif

#endif
