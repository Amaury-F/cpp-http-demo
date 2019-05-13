#ifndef TP4_COMMMETHODS_H
#define TP4_COMMMETHODS_H

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <string>
#define SECURE

#ifdef SECURE

    #define CERT "mycert.pem"
    #define KEY "mycert.pem"

    inline SSL_CTX *_create_ssl_context(const std::string& source) {
        SSL_library_init();
        const SSL_METHOD *method;
        SSL_CTX *ctx;
        OpenSSL_add_all_algorithms();
        SSL_load_error_strings();

        if (source == "server") {
            method = SSLv23_server_method();
        } else {
            method = SSLv23_client_method();
        }

        ctx = SSL_CTX_new(method);
        if (ctx == nullptr) {
            perror("Unable to create SSL context");
            ERR_print_errors_fp(stderr);
            exit(EXIT_FAILURE);
        }
        return ctx;
    }

    inline void _configure_ssl_context(SSL_CTX *ctx) {
        SSL_CTX_set_ecdh_auto(ctx, 1);

        /* Set the key and cert */
        if (SSL_CTX_use_certificate_file(ctx, CERT, SSL_FILETYPE_PEM) <= 0) {
            ERR_print_errors_fp(stderr);
            exit(EXIT_FAILURE);
        }

        if (SSL_CTX_use_PrivateKey_file(ctx, KEY, SSL_FILETYPE_PEM) <= 0 ) {
            ERR_print_errors_fp(stderr);
            exit(EXIT_FAILURE);
        }
    }

    struct _ssl_sock_t {SSL* ssl; int socket;};
    typedef struct _ssl_sock_t reply_sock_t;

    inline reply_sock_t _make_ssl_reply_fd(SSL_CTX *ctx, int client_fd) {
        reply_sock_t reply_fd{};
        reply_fd.ssl = SSL_new(ctx);
        reply_fd.socket = client_fd;
        SSL_set_fd((reply_fd).ssl, (reply_fd).socket);

        return reply_fd;
    }

    inline int _context_ssl_connect(struct _ssl_sock_t reply_fd) {
        return SSL_connect((reply_fd).ssl);
    }

    inline int _context_ssl_accept(struct _ssl_sock_t reply_fd) {
        return SSL_accept((reply_fd).ssl);
    }


    typedef SSL_CTX context_t;
    #define create_context(source) _create_ssl_context(source)

    #define configure_context(ctx) _configure_ssl_context(ctx)

    #define make_reply_fd(context, socket_fd) \
            _make_ssl_reply_fd(context, socket_fd)

    #define context_connect(reply_fd) \
            _context_ssl_connect(reply_fd)

    #define context_accept(reply_fd) \
            _context_ssl_accept(reply_fd)

    #define delete_context(context) SSL_CTX_free(ctx)

    #define socket_write(reply_fd, buf, len) \
            SSL_write((reply_fd).ssl, buf, (int) len)

    #define socket_read(reply_fd, buf, len) \
            SSL_read((reply_fd).ssl, buf, (int) len)

    #define reply_close(reply_fd)      \
            SSL_free((reply_fd).ssl);  \
            close((reply_fd).socket);


    #define context_perror() ERR_print_errors_fp(stderr);

#else

    typedef int reply_sock_t;
    typedef void context_t;
    #define create_context(source) NULL

    #define configure_context(context) NULL

    #define make_reply_fd(context, client_fd) (client_fd)

    #define context_connect(reply_fd) true
    #define context_accept(reply_fd) true

    #define delete_context(context) NULL

    #define socket_write write
    #define socket_read read

    #define reply_close(reply_fd) close(reply_fd)

    #define context_perror()

#endif


#endif //TP4_COMMMETHODS_H
