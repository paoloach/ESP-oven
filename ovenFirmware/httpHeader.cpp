#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "httpHeader.h"

HttpHeader::HttpHeader(){
  header=NULL;
  value = NULL;
}

HttpHeader::HttpHeader(HttpHeader & other) {
  header = other.header;
  value = other.value;
  other.header=NULL;
  other.value = NULL;
}

HttpHeader::HttpHeader(const HttpHeader & other) {
  if (other.header != NULL){
    header = strdup(other.header);
  }
  if (other.value != NULL){
    value = strdup(other.value);
  }
}

HttpHeader::HttpHeader(const char * line) {
  const char * iter= line;
  const char * startValue=NULL;
  char c = iter[0];
  char c1 = iter[1];
  while (!(c=='\r' && c1 == '\n')){
    if (c == ':'){
      startValue=iter+1;
    }
    c=c1;
    iter++;
    c1=iter[1];
  }
  int lenHeader;
  if (startValue != NULL){
    lenHeader = startValue-line;
  } else {
    lenHeader = iter-line;
  }
  if (lenHeader>0){
    if (lenHeader > 20){
      lenHeader=20;
    }
    header = new char[lenHeader+1];
    memcpy(header, line, lenHeader);
    header[lenHeader]=0;
  }
  if (startValue != NULL){
    int lenValue = iter-startValue;
    if (lenValue>0){
      value = new char[lenValue+1];
      memcpy(value, startValue, lenValue);
      value[lenValue]=0;
    }
  }
}
HttpHeader::~HttpHeader(){
  printf("[%s:%d]\n", __FILE__, __LINE__);
  delete header;
  delete value;
}

void HttpHeader::print(){
  printf("%s=",header);
  if (value != NULL){
    printf(value);
  }
}
