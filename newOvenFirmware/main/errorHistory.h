#ifndef __ERROR_HISTORY__H__
#define __ERROR_HISTORY__H__

#include <stdint.h>
#include "ovenConfig.h"

class ErrorHistory {
public:
  ErrorHistory():nextToAdd(0){}

  void addError(int err);
  int32_t sumLast(uint32_t lastCount);
  uint32_t maxMeanError();
  uint32_t getSize() const {return nextToAdd;}
private:
  int32_t errors[MAX_ERROR_HYSTORY];
  uint32_t nextToAdd;
};

#endif
