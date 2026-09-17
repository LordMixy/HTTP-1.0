#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include "http_header.h"
#include <stddef.h> 
#include <stdint.h> 

#define MAX_ROUTES 50

typedef struct http_transaction (*http_callback)([[maybe_unused]] struct http_header*);

enum http_method {
	M_GET, M_POST, M_HEAD
};
 
struct http_transaction {
	struct http_header* headers;
	const char* entity_body;
};

struct http_route {
	const char* route_path; 
	http_callback callback[3]; // GET, POST, HEAD  
};

struct http_server {
	const char* port;
	const char* root_dir;
	struct http_route routes[MAX_ROUTES];
	size_t routes_count;
};

int32_t http_server_route(struct http_server server[static 1], const char route[static 1], 
						  enum http_method method, http_callback callback);
						  
int32_t transaction_set_body(struct http_transaction transaction[static 1], const char content[static 1]);

int32_t http_server_listen(struct http_server server[static 1]);

[[nodiscard]] struct http_header* parse_request(const char request[static 1]);

[[nodiscard]] char* transaction_to_string(struct http_transaction transaction[static 1]);

[[nodiscard]] char* handle_request(struct http_server server[static 1], const char buffer[static 1]);

#endif // HTTP_SERVER_H
