#ifndef __URL_MATCHER__H__
#define __URL_MATCHER__H__

#include <vector>
#include <functional>
#include "Connections.h"

class UrlMatcher {
public:
  UrlMatcher(const char * matcher, std::function<void( Connection *)>  f);
  UrlMatcher(UrlMatcher && other):matcher(other.matcher), function(other.function){other.matcher=nullptr;function=nullptr;}
  UrlMatcher(const UrlMatcher & other)=delete;
  UrlMatcher():matcher(nullptr){}
  ~UrlMatcher(){delete matcher;}
  bool operator==(const char *);
  void operator()(Connection * c);
private:
  char * matcher;
  std::function<void( Connection *)> function;
};

#endif
