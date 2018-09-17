#include <stdio.h>
#include <string.h>
#include "PID.h"

PID::PID(ErrorHistory & errorHistory): errorHistory(errorHistory){
  propCoef=10;
  integralCoef=1;
}

int PID::calc(int err){
  if (err < 0){
    return 0;
  }

  int integral = errorHistory.sumLast(MAX_INTEGRAL_SIZE);
  int u = err*propCoef + integral*integralCoef;
  printf("u=%d (err=%d, integral=%d) \n", u, err, integral);
  if (u > 100){
    u=100;
  } else if (u<0){
    u=0;
  }
  return u;
}
