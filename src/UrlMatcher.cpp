
#include <string.h>
#include <stdio.h>
#include "UrlMatcher.h"

UrlMatcher::UrlMatcher(const char * url, std::function<void( Connection *) > f):function(f){
  int len = strlen(url)+1;
  matcher = new char[len];
  memcpy(matcher, url, len);
}
bool UrlMatcher::operator==(const char * url){
  return strcmp(matcher, url)==0;
}

void UrlMatcher::operator()(Connection * c){
  if (function)
    function(c);
  else
    printf("invalid function for %s\n",matcher );
}
