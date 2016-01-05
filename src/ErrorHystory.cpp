#include <string.h>
#include <stdlib.h>
#include "ErrorHistory.h"

void ErrorHistory::addError(int error){
  if (nextToAdd < MAX_ERROR_HYSTORY){
    errors[nextToAdd] = error;
    nextToAdd++;
  } else {
    memmove(errors, errors+1,MAX_ERROR_HYSTORY-2 );
    errors[MAX_ERROR_HYSTORY-1]=error;
  }
}

int ErrorHistory::sumLast(uint32_t lastCount) {
  int sum=0;
  const int32_t *iter;
  const int32_t *end;
  if (nextToAdd < lastCount){
    iter = errors;
    end = errors+nextToAdd;
  } else {
    end = errors+MAX_ERROR_HYSTORY;
    iter = end-lastCount;
  }
  for (;iter < end; iter++){
    sum += *iter;
  }

  return sum;
}

uint32_t ErrorHistory::maxMeanError(int threshold) {
  uint32_t maxErr=0;
  const int * iter=errors;
  const int * end=errors+nextToAdd;
  for(;iter < end; iter++){
    uint32_t err = abs(threshold - *iter);
    if (err > maxErr){
      maxErr = err;
    }
  }
  return maxErr;
}
