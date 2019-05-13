#include <sstream>
#include <iostream>
#include <cstring>
#include "HttpParser.h"


HttpRequest HttpParser::parse(const char *request) {
    HttpRequest res;

    if (memcmp(request, "GET", 3) == 0) {
        res.method = HttpRequest::GET;
    } else if (memcmp(request, "POST", 4) == 0) {
        res.method = HttpRequest::POST;
    } else {
        res.method = HttpRequest::NOT_IMPLEMENTED;
        res.status = 405;
        res.explanation = "Method not allowed";
        return res;
    }

    const char * p = request + 5 + (res.method == HttpRequest::POST);
    const char * q = p;
    for (p; *p != ' '; ++p);
    res.filename = std::string(q, p - q);

    for (p; *(p+2) != '\n'; ++p);
    res.version = *p - '0';

    time_t rawtime;
    time(&rawtime);
    char ti[50];
    strftime(ti, 50, "%a, %d %b %Y %T %Z", localtime(&rawtime));
    res.date = std::string(ti);

    return res;
}


size_t HttpParser::makeResponse(HttpRequest request, char *dest) {
    HttpParser::nbr_use += 1;

    std::stringstream buf;
    buf << "HTTP/1." << request.version << " " << request.status << " " << request.explanation << "\r\n";
    buf << "Content-Length: " << request.filelen << "\r\n";
    buf << "Date: " << request.date << "\r\n";
    buf << "Connection: close\r\n";
    buf << "Set-Cookie: nbr_use=" << HttpParser::nbr_use << "; Max-Age=3600\r\n";
    buf << "\r\n";
    buf.seekg(0, std::ios::end);
    strcpy(dest, buf.str().c_str());

    return (size_t) buf.tellg();
}



