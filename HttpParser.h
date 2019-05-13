#ifndef TP1_HTTPPARSER_H
#define TP1_HTTPPARSER_H

#include "HttpRequest.h"

class HttpParser {
public:
    static HttpRequest parse(const char * request);
    static size_t makeResponse(HttpRequest request, char * dest);

private:
    HttpParser() = default;
    static inline int nbr_use = 0;
};


#endif //TP1_HTTPPARSER_H
