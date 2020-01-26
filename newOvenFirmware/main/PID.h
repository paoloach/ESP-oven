#ifndef __PID__H__
#define __PID__H__

#include <stdint.h>
#include "ovenConfig.h"
#include "errorHistory.h"


class PID {
public:
  PID(ErrorHistory & errorHistory);
  double propCoef;
  double integralCoef;
  int calc(int error);
private:
  ErrorHistory & errorHistory;
};

#endif
