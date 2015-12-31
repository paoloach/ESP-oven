#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "http.h"

HTTP::HTTP(){
  status = HTTP::METHOD;
  httpVersion = HTTP_UNKNOWN_VERSION;
  parseError = false;
  url = nullptr;
  body = nullptr;
  needOtherData=true;
}

HTTP::~HTTP(){
  delete url;
  delete body;
}

void HTTP::init(){
  delete url;
  delete body;
  status = HTTP::METHOD;
  httpVersion = HTTP_UNKNOWN_VERSION;
  parseError = false;
  url = nullptr;
  body = nullptr;
  bodyLen=0;
  needOtherData=true;
}
void HTTP::feed(const char * data, u16_t len){
  const char * endData = data + len;
  const char * curData = data;
  const char * nextToken;
  needOtherData=false;
  while (curData < endData && !parseError && !needOtherData){
      switch (status) {
      case METHOD:
        curData = parseMethod(curData, endData);
        curData = removeSpaces(curData, endData);
        status = REQUEST_URI;
        break;
      case REQUEST_URI:
        nextToken = findSpace(curData, endData);
        if (nextToken <  endData){
          int size = nextToken - curData;
          url = new char[size+1];
          memcpy(url, curData, size);
          url[size] = 0;
          curData = removeSpaces(nextToken, endData);;
          status = HTTP_VERSION;
        } else {
          needOtherData=true;
        }
        break;
      case HTTP_VERSION:
        nextToken = findEndLine(curData, endData);
        if (nextToken <  endData){
          if (nextToken - curData >= 8){
            if (memcmp(curData,"HTTP/1.1", 8)==0){
              httpVersion = HTTP_1_1;
              printf("http Version 1.1\n");
            } else if (memcmp(curData,"HTTP/1.0",8)==0){
              httpVersion = HTTP_1_0;
              printf("http Version 1.0\n");
            } else {
              httpVersion = HTTP_UNKNOWN_VERSION;
              parseError=true;
              printf("unknown version\n");
            }
          } else {
            httpVersion = HTTP_UNKNOWN_VERSION;
            parseError=true;
            printf("unknown version\n");
          }
          curData = nextToken;
          status = HTTP_HEADERS;
        } else {
          printf("Need other data\n");
          needOtherData=true;
        }
        break;
      case HTTP_HEADERS:
        nextToken = findEndLine(curData, endData);
        if (nextToken <  endData){
          if (nextToken - curData > 2){
            headers.emplace_back(curData);
            curData = nextToken;
            if (curData[0] == '\r' && curData[1] == '\n'){
                status = HTTP_BODY;
                curData += 2;
            }
          } else {
            printf("End parse\n");
            status=END_PARSE;
          }
        } else {
          needOtherData=true;
        }
        break;
      case HTTP_BODY:
        bodyLen = endData - curData;
        if (bodyLen > 0){
          body = new char[bodyLen+1];
          memcpy(body, curData, bodyLen);
          body[bodyLen] = 0;
        }
        status = END_PARSE;
        break;
      case END_PARSE:
        print();
        return;
        break;
      case PARSE_ERROR:
        printf("Parse error\n");
        return;
        break;
    }
    if (parseError){
      status = PARSE_ERROR;
    }
  }
  print();
}

void HTTP::print(){
  printf("method: %d\n", method);
  printf("url: %s\n", url );
  printf("headers found: %d\n", headers.size());
  if (body != nullptr){
    printf("body: %s\n", body);
  }

  for(auto & header: headers){
    header.print();
    printf("\n" );
  }

}

const char * HTTP::findEndLine(const char * data, const char * endData) {
  while((data+1) < endData && (data[0] != '\r' || data[1] != '\n')){
    data++;
  }
  return data+2;
}

const char * HTTP::removeSpaces(const char * data, const char * endData) {
  while(isspace(*data) && data < endData){
    data++;
  }
  return data;
}

const char * HTTP::findSpace(const char * data, const char * endData) {
  while(!isspace(*data) && data < endData){
    data++;
  }
  return data;
}

const char * HTTP::parseMethod(const char * data, const char * endData){
  u16_t len = endData - data;

  if (len >= 7 && memcmp(data, "OPTIONS",7)==0){
    data += 7;
    method = Method::OPTIONS;
  } else if (len >= 4 && memcmp(data, "GET",3)==0){
    data += 4;
    method = Method::GET;
  } else if (len >= 5 && memcmp(data, "HEAD",4)==0){
    data += 5;
    method = Method::HEAD;
  }else if (len >= 5 && memcmp(data, "POST",4)==0){
    data += 5;
    method = Method::POST;
  }else if (len >= 4 && memcmp(data, "PUT",3)==0){
    data += 4;
    method = Method::PUT;
  }else if (len >= 7 && memcmp(data, "DELETE",6)==0){
    data += 7;
    method = Method::DELETE;
  }else if (len >= 6 && memcmp(data, "TRACE",5)==0){
    data += 6;
    method = Method::TRACE;
  }else if (len >= 7 && memcmp(data, "CONNECT",7)==0){
    data += 7;
    method = Method::CONNECT;
  } else {
    method = Method::UNDEFINED_METHOD;
    parseError = true;
  }

  return data;
}
