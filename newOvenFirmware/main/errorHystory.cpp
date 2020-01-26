#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "errorHistory.h"

void ErrorHistory::addError(int error){
  if (nextToAdd < MAX_ERROR_HYSTORY){
    errors[nextToAdd] = error;
    nextToAdd++;
  } else {
    int * iter=errors;
    const int * source=errors+1;
    const int * end=errors+MAX_ERROR_HYSTORY;
    for(;source < end; source++, iter++){
      *iter = *source;
    }
    errors[MAX_ERROR_HYSTORY-1]=error;
  }
//   const int * iter=errors;
//   const int * end=errors+nextToAdd;
//   printf("h: ");
//   for(;iter < end; iter++){
//     printf("%d, ", *iter);
//   }
//   printf("\n");
}

int ErrorHistory::sumLast(uint32_t lastCount) {
  int sum=0;
  const int32_t *iter;
  const int32_t *end;
  if (nextToAdd < lastCount){
    iter = errors;
    end = errors+nextToAdd;
  } else {
    end = errors+nextToAdd;
    iter = end-lastCount;
  }
  for (;iter < end; iter++){
    sum += *iter;
  }

  return sum;
}

uint32_t ErrorHistory::maxMeanError() {
  uint32_t maxErr=0;
  const int * iter=errors;
  const int * end=errors+nextToAdd;
  for(;iter < end; iter++){
    uint32_t err = abs(*iter);
    if (err > maxErr){
      maxErr = err;
    }
  }
  return maxErr;
}
