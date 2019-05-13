#ifndef TP1_HTTPREQUEST_H
#define TP1_HTTPREQUEST_H

#include <string>

class HttpRequest {
public:
    typedef enum {
        GET, POST, NOT_IMPLEMENTED
    } MethodType;

    MethodType method;
    int version;
    std::string filename;
    int status;
    std::string explanation;
    long int filelen;
    std::string date;

    HttpRequest() = default;

};


#endif //TP1_HTTPREQUEST_H
