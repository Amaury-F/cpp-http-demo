cmake_minimum_required(VERSION 3.9)
project(https-demo)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")


add_executable(client-http client.cpp CommMethods.h)
target_link_libraries(client-http ssl crypto)

add_executable(server-http server.cpp HttpThread.cpp HttpThread.h HttpParser.cpp HttpParser.h HttpRequest.h CommMethods.h)
target_compile_definitions(server-http PUBLIC DEFAULT_PORT=80)
target_link_libraries(server-http pthread ssl crypto)

add_executable(client-https client.cpp CommMethods.h)
target_compile_definitions(client-https PUBLIC SECURE)
target_link_libraries(client-https ssl crypto)

add_executable(server-https server.cpp HttpThread.cpp HttpThread.h HttpParser.cpp HttpParser.h HttpRequest.h CommMethods.h)
target_compile_definitions(server-https PUBLIC SECURE DEFAULT_PORT=443)
target_link_libraries(server-https pthread ssl crypto)

