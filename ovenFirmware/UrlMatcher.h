#ifndef __URL_MATCHER__H__
#define __URL_MATCHER__H__

#include <vector>
#include "http.h"


typedef void (*UrlMatcherCB)(int fd, HTTP & http);

class UrlMatcher {
public:
  UrlMatcher(const char * matcher, UrlMatcherCB  f);
  UrlMatcher(UrlMatcher & other):matcher(other.matcher), function(other.function){other.matcher=nullptr;function=nullptr;}
  UrlMatcher(const UrlMatcher & other);
  UrlMatcher():matcher(nullptr){}
  ~UrlMatcher(){delete matcher;}
  bool operator==(const char *);
  void operator()(int fd, HTTP & http);
private:
    
  char * matcher;
  UrlMatcherCB function;
};

#endif
