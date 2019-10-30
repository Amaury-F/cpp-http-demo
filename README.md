# HTTP/HTTPS file server and client written in C/C++.

## Server

Listen and parse HTTP requests with C TCP sockets, then send the requested file in a separate thread (unix pthread) in buffers. Works seemlessly with a web browser.

There is also a HTTPS version, which handles the handshake and encrypts the file with the OpenSSL C++ library.


## Client

Emmits an HTTP request to the specified adress with a TCP socket, and prints the received file. The HTTPS client uses OpenSSL for the handshake and to decrypt the received files.
