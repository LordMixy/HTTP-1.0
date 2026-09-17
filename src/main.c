#define _GNU_SOURCE

#include "../include/http_server.h"
#include <stdio.h>

struct http_transaction get_home([[maybe_unused]] struct http_header* header)
{
	struct http_transaction res = { .headers = NULL };

	header_set(&res.headers, "Content-Type", "text/html");
	transaction_set_body(&res, "LA vioe!");
	
	return res;
}

int main([[maybe_unused]] int argc, char* argv[])
{
    struct http_server server = {
        argv[1],
        "./",
		{ { 0 } },
		.routes_count = 0
    };

   	http_server_route(&server, "/", M_GET, get_home);
	// http_server_static(&server, "/");
		
    return http_server_listen(&server);
}
