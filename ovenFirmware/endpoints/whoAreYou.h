//
// Created by paolo on 12/09/18.
//

#ifndef OVENFIRMWARE_WHOAREYOU_H
#define OVENFIRMWARE_WHOAREYOU_H

static char *const RESPONSE = "I am ESP-OVEN\n";

#include <c_types.h>


#include "../httpd/include/httpd.h"

int cgiWhoAreYou(HttpdConnData *connData);
#endif //OVENFIRMWARE_WHOAREYOU_H
