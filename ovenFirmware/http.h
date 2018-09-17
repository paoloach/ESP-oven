#ifndef __HTTP__H__
#define __HTTP__H__

#include <vector>
#include "lwip/arch.h"
#include "httpHeader.h"

enum Method {
  UNDEFINED_METHOD=-1,
  OPTIONS,
  GET,
  HEAD,
  POST,
  PUT,
  DELETE,
  TRACE,
  CONNECT
};

class HTTP {
public:
  HTTP();
  ~HTTP();
public:
  void feed(const char * data, u16_t len);
  bool error() const {
    return status == PARSE_ERROR;
  }
  bool needMoreData() const {
    return status != END_PARSE && status != PARSE_ERROR;
  }
  Method getMethod() const {
    return method;
  }
  const char * getUrl() const {
    return url;
  }
  const char * getBody() const {
    return body;
  }
  int getBodyLength()const {
    return bodyLen;
  }
  void print();
private:
  enum Status {
    PARSE_ERROR=-1,
    FIRST_LINE,
    HTTP_HEADERS,
    HTTP_BODY,
    END_PARSE
  };



  enum HttpVersion {
    HTTP_UNKNOWN_VERSION=-1,
    HTTP_1_0,
    HTTP_1_1
  };

  Status status;
  Method method;
  HttpVersion httpVersion;
  char * url;
  char * body;
  char * bufferIter;
  int bodyLen;
  std::vector<HttpHeader> headers;

  const char * parseMethod(const char * data);
  const char * removeSpaces(const char * data);
  const char * findSpace(const char * data);
  void feedNewLine();
};

#endif
