#ifndef __HTTP_HEADER__H__
#define __HTTP_HEADER__H__

class HttpHeader {
public:
  HttpHeader();
  HttpHeader(const HttpHeader & other);
  HttpHeader(HttpHeader & other);
  HttpHeader(const char * line);
  ~HttpHeader();
  void print();
  char * header;
  char * value;
};


#endif
