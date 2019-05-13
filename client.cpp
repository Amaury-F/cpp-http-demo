#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <iostream>
#include <unordered_map>
#include <csignal>
#include "CommMethods.h"

#define F_BOLD "\x1B[1m"
#define F_END "\x1B[0m"

#define PORT_MAX 65535
#define PORT_MIN 1

#define BUF_SIZE 4096
#define NAME_LEN_MAX 64

void error(const char *msg) {
    perror(msg);
    context_perror();
    exit(0);
}

static volatile bool keepRunning = true;
void intHandler(int) {
    keepRunning = false;
}

int main(int argc, char *argv[]) {
    int sockfd = 0;
    uint16_t portno;
    struct sockaddr_in serv_addr{};
    struct hostent *server;
    std::unordered_map<std::string, std::string> cookies;

    if (argc < 3) {
        fprintf(stderr,"usage %s hostname port\n", argv[0]);
        exit(0);
    }

    context_t *ctx = create_context("client");

    // convert port arg into port number.
    long int argport = strtol(argv[2], nullptr, 0);
    if (argport > PORT_MAX || argport < PORT_MIN) {
        error("Port is out of bounds.");
    }
    portno = (uint16_t) argport;

    server = gethostbyname(argv[1]);
    if (server == nullptr) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }

    // make server adress from host_by_name
    memset((char *) &serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(portno);
    memcpy((char *) &serv_addr.sin_addr.s_addr, server->h_addr, (size_t) server->h_length);


    //set interruption signal handler
    struct sigaction action{};
    action.sa_handler = intHandler;
    action.sa_flags = 0;
    sigemptyset( &action.sa_mask );
    sigaction(SIGINT, &action, nullptr);

    char buffer[BUF_SIZE]; // requests and answers
    char filename[NAME_LEN_MAX]; // path of the requested file
    std::cout << F_BOLD << "\nPress Ctrl+D to quit.\n" << F_END;
    std::cout << F_BOLD << "\nEnter file path : " << F_END;
    while (scanf("%s", filename) == 1 && keepRunning) {

        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0) {
            error("ERROR opening socket");
        }
        if (connect(sockfd,(struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
            error("ERROR connecting");
        }

        reply_sock_t reply_fd = make_reply_fd(ctx, sockfd);
        if (!context_connect(reply_fd)) {
            error("ERROR connecting in context");
        }

        // make request header
        sprintf(buffer, "GET /%s HTTP/1.1\r\n"
                "Host: %s\r\n"
                "Connection: close\r\n\r\n", filename, argv[1]);

        // send request to the server
        if (socket_write(reply_fd, buffer, strlen(buffer)) < 0) {
            error("ERROR writing to socket");
        }
        memset(buffer, 0, BUF_SIZE);



        std::cout << F_BOLD <<"\nServer Response :\n" << F_END;
        std::string cookie_name;
        bool new_cookie = false;
        while (socket_read(reply_fd, buffer, BUF_SIZE - 1) > 0) { //answer can be in several buffers
            std::cout << buffer;

            // search "Set-Cookie" instruction
            const char *p = strstr(buffer, "Set-Cookie");
            if (p != nullptr) {
                new_cookie = true;
                p += 12;
                const char *q = strstr(p, "=");  //           *p  *q    *s
                const char *s = strstr(q, ";");  //            |   |     |
                cookie_name = std::string(p, q); //Set-Cookie: name=value;
                std::string value = std::string(q + 1, s - q);
                cookies[cookie_name] = value; // add new cookie to hash map
            }

            memset(buffer, 0, BUF_SIZE);
        }

        reply_close(reply_fd);

        // display new cookie after whole answer is read
        if (new_cookie) {
            std::cout << std::endl << F_BOLD << "New cookie : " << F_END <<
                      cookie_name << " = " << cookies[cookie_name] << std::endl;
        }
        std::cout << F_BOLD << "\nEnter file path : " << F_END;
    }

    std::cout << std::endl;
    close(sockfd);
    delete_context(ctx);
    return 0;
}

