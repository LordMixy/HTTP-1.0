#ifndef HTTP_HEADER_H
#define HTTP_HEADER_H

#include <stdint.h>

#define HTTP_VERSION          "HTTP/1.0"

#define OK                    "200"
#define CREATED               "201"
#define ACCEPTED              "202"
#define NO_CONTENT            "204"
#define MOVED_PERMANENTLY     "301"
#define MOVED_TEMPORARILY     "302"
#define NOT_MODIFIED          "304"
#define BAD_REQUEST           "400"
#define UNAUTHORIZED          "401"
#define FORBIDDEN             "403"
#define NOT_FOUND             "404"
#define INTERNAL_SERVER_ERROR "500"
#define NOT_IMPLEMENTED       "501"
#define BAD_GATEWAY           "502"
#define SERVICE_UNAVAILABLE   "503"

struct http_header { 
	char key[50];
	char value[100];
	struct http_header* left;
	struct http_header* right;
};

[[nodiscard]] 
char* header_get(struct http_header map[static 1], char key[static 1]);

int32_t header_set(struct http_header* map[static 1], char key[static 1], char value[static 1]);

void header_free(struct http_header* map);

#endif // HTTP_HEADER_H
