#ifndef TP1_HTTPTHREAD_H
#define TP1_HTTPTHREAD_H


#include "CommMethods.h"

class HttpThread {
public:
    /**
     * @param sockfd : File descriptor of the client socket previously accepted.
     *                 Must be dinamically allocated.
     */
    explicit HttpThread(reply_sock_t sockfd);
    ~HttpThread();

    int run();

private:
    reply_sock_t sockfd;
};


#endif //TP1_HTTPTHREAD_H
