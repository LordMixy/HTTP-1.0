cmake_minimum_required(VERSION 3.22)
project(httpsrv LANGUAGES C)

set(CMAKE_C_STANDARD 23)
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

add_library(http_server STATIC ./src/http_server.c ./include/http_server.h)
add_library(http_header STATIC ./src/http_header.c ./include/http_header.h)

add_executable(httpsrv ./src/main.c)
target_link_libraries(httpsrv PUBLIC http_server)
target_link_libraries(httpsrv PUBLIC http_header)
target_compile_options(httpsrv PUBLIC -fstack-usage -std=c2x -Wpedantic -Wall -Wextra -Werror)
