
#include <string.h>
#include <stdio.h>
#include "esp_log.h"
#include "UrlMatcher.h"

UrlMatcher::UrlMatcher(const char *url, UrlMatcherCB f) : function(f) {

    int len = strlen(url) + 1;
    matcher = new char[len];
    memcpy(matcher, url, len);
}

UrlMatcher::UrlMatcher(const UrlMatcher &other) {
    int len = strlen(other.matcher) + 1;
    matcher = new char[len];
    memcpy(matcher, other.matcher, len);
    function = other.function;
}

bool UrlMatcher::operator==(const char *url) {
    return strcmp(matcher, url) == 0;
}

void UrlMatcher::operator()(int fd, HTTP &http) {
    if (function)
        function(fd, http);
    else
        ESP_LOGE("http","invalid function for %s", matcher);
}

UrlMatcher::UrlMatcher(UrlMatcher &&other) {
    matcher = other.matcher;
    function = other.function;
    other.matcher = nullptr;
    other.function= nullptr;
}


