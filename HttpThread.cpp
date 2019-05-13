#include "HttpThread.h"
#include "CommMethods.h"
#include "HttpRequest.h"
#include "HttpParser.h"
#include <malloc.h>
#include <cstring>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <sstream>

#define BUF_SIZE 4096

HttpThread::HttpThread(reply_sock_t sockfd) {
    this->sockfd = sockfd;
}

HttpThread::~HttpThread() = default;

void *handle_exchange(reply_sock_t *sockfd);
int HttpThread::run() {
    pthread_t thr;
    // create thread and call routine for the request
    reply_sock_t *arg = (reply_sock_t *) malloc(sizeof(sockfd));
    memcpy(arg, &sockfd, sizeof(sockfd));
    int r = pthread_create(&thr, nullptr, (void *(*)(void *)) handle_exchange, arg);
    return r;
}

void *handle_exchange(reply_sock_t *sockfd) {
    ssize_t n;
    char buffer[BUF_SIZE];
    memset(buffer, 0, BUF_SIZE);
    // read request, should read in a loop because the exchange may not be received in one call to socket_read()
    n = socket_read(*sockfd, buffer, BUF_SIZE - 1);
    if (n < 0) {
        perror("ERROR reading from socket");
        return nullptr;
    }

    // make request object from the received buffer
    HttpRequest req = HttpParser::parse(buffer);
    if (req.method == HttpRequest::NOT_IMPLEMENTED) {
        std::cerr << "METHOD NOT IMPLEMENTED IN REQUEST :" << std::endl;
        std::cerr << buffer << std::endl;
    }

    // try to open requested file
    std::ifstream file;
    file.open(req.filename, std::ios::binary | std::ios::ate);
    // update the request infos based on opened file
    if (!file.is_open()) {
        req.status = 404;
        req.explanation = "Not found";
    } else {
        req.status = 200;
        req.explanation = "OK";
    }
    req.filelen = file.tellg();
    std::cout << req.filename << " : " << req.status << " " << req.explanation << "; size:" << req.filelen << "\n";

    //make a header for the answer and send it
    size_t headerlen = HttpParser::makeResponse(req, buffer);
    if (socket_write(*sockfd, buffer, headerlen) < 0) {
        perror("ERROR writing to socket");
        return nullptr;
    }

    // read the whole file and send it in several buffers
    long int toread = req.filelen;
    file.seekg(0, std::ios::beg);
    while (toread > 0) {
        file.read(buffer, BUF_SIZE < toread ? BUF_SIZE : toread);
        long int r = file.gcount();
        toread -= r;

        n = socket_write(*sockfd, buffer, (size_t) r);
        if (n < 0) {
            perror("ERROR writing to socket");
            return nullptr;
        }
    }

    // free the resources
    file.close();
    reply_close(*sockfd);
    free(sockfd);
    pthread_detach(pthread_self());
    return nullptr;
}
