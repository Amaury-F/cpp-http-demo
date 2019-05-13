#include <cstdio>
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <csignal>
#include "CommMethods.h"
#include "HttpThread.h"

#define PORT_MAX 65535
#define PORT_MIN 1
#define QUEUE_LIMIT 10

#ifndef DEFAULT_PORT
#define DEFAULT_PORT ""
#endif

#define error(message) \
    perror(message);   \
    context_perror();  \
    return EXIT_FAILURE;


static volatile bool keepRunning = true;
void intHandler(int) {
    keepRunning = false;
}

int main(int argc, char *argv[])  {
    int servfd;
    uint16_t port;
    socklen_t client_len;

    struct sockaddr_in serv_addr{}, cli_addr{};


    servfd = socket(AF_INET, SOCK_STREAM, 0);
    if (servfd < 0) {
        error("Could not open socket");
    }
    int opt = 1;
    setsockopt(servfd, SOL_SOCKET, SO_REUSEADDR, (void*) &opt, sizeof (int));

    //convert port arg into port number
    if (argc >= 2) {
        long int argport = strtol(argv[1], nullptr, 0);
        if (argport > PORT_MAX || argport < PORT_MIN) {
            std::cerr << "Port " << argport << " is out ouf bounds." << DEFAULT_PORT << " will be used by default.\n";
            port = DEFAULT_PORT;
        } else {
            port = (uint16_t) argport;
        }
    } else {
        std::cout << "Port " << DEFAULT_PORT << " will be used by default.\n";
        port = DEFAULT_PORT;
    }


    //make addr
    memset((char *) &serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port);

    if (bind(servfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        error("Could not bind server socket");
    }
    if (listen(servfd, QUEUE_LIMIT) < 0) {
        error("listen");
    }

    context_t *ctx = create_context("server");
    configure_context(ctx);

    //set interruption signal handler
    struct sigaction action{};
    action.sa_handler = intHandler;
    action.sa_flags = 0;
    sigemptyset( &action.sa_mask );
    sigaction(SIGINT, &action, nullptr);

    while (true) {
        client_len = sizeof(cli_addr);

        // connect to client
        int client_fd = accept(servfd, (struct sockaddr *) &cli_addr, &client_len);
        if (!keepRunning)  {
            break;
        } else if (client_fd < 0) {
            error("Could not accept handshake");
        }

        //prepare reply socket
        reply_sock_t reply_fd = make_reply_fd(ctx, client_fd);
        if (!context_accept(reply_fd)) {
            error("Could not accept context-based connexion");
        }

        // run separate thread for the request
        HttpThread thread(reply_fd);
        int r = thread.run();
        if (r < 0) {
            error("Could not run HTTP exchange");
        }

    }

    close(servfd);
    delete_context(ctx);
    return 0;
}
