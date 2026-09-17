#define _GNU_SOURCE

#include "../include/http_header.h"
#include "../include/http_server.h"
#include "../include/debug.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <memory.h>
#include <unistd.h>
#include <poll.h>
#include <sys/epoll.h>

#define CLIENT_POLL_SIZE 1000
#define BUFF_SIZE        1024
#define BACKLOG          10

struct poll {
	struct pollfd clients[CLIENT_POLL_SIZE];
	size_t size;
};

static struct poll c_poll = { 0 };

static inline 
void push_poll(int32_t client)
{
	c_poll.clients[c_poll.size].fd = client;
	c_poll.clients[c_poll.size].events = POLLIN;
	c_poll.size++;
}

static inline
void parse_request_line(struct http_header** header, char* line)
{
	char* request_line;
	header_set(header, "Method", strtok_r(line, " ", &request_line));
	header_set(header, "Request-URI", strtok_r(NULL, " ", &request_line));
	header_set(header, "HTTP-Version", strtok_r(NULL, " ", &request_line));
}

static inline
void header_line_split(char* header_line, char* token[static 2])
{
	char* save;
	token[0] = strtok_r(header_line, ":", &save);
	token[1] = strtok_r(NULL, "\r\n", &save);
}

struct http_header* parse_request(const char request[static 1])
{
	struct http_header* header = NULL;
	
	// parsing della prima linea della richiesta http 
	char* save;
	char* line = strtok_r((char*) request, "\r\n", &save);
	parse_request_line(&header, line);
	
	// si passa alla seconda
	line = strtok_r(NULL, "\r\n", &save);

	// parsing...
	char* token[2];
	while (line) { 
		header_line_split(line, token);	
		header_set(&header, token[0], token[1]);		
		line = strtok_r(NULL, "\r\n", &save);
	}
	
	return header;
}

[[nodiscard]] static inline
int32_t get_route(struct http_server* server, const char* route_path)
{
	int32_t route = -1;
	for (size_t i = 0; i < server->routes_count; ++i) {
		if (!strcmp(server->routes[i].route_path, route_path)) {
			route = i;
			break;
		}
	}
	return route;
}

int32_t transaction_set_body(struct http_transaction transaction[static 1], const char content[static 1])
{
	transaction->entity_body = content;
	
	char content_length[10];
	sprintf(content_length, "%lu", strlen(content));
	
	return header_set(&transaction->headers, "Content-Length", content_length);	
}

#define E_TOO_MANY_ROUTES -1

int32_t http_server_route(
	struct http_server server[static 1], 
	const char         route_path[static 1], 
	enum http_method   method, 
	http_callback      callback)
{
	int32_t route_index = get_route(server, route_path);

	if (route_index == -1) { 
		if (server->routes_count == MAX_ROUTES) {
			debug_error("CANNOT PUSH ROUTE %s\n", route_path);
			return E_TOO_MANY_ROUTES;
		}

		debug("PUSHING ROUTE %s\n", route_path);
		server->routes[server->routes_count].route_path = route_path;
		server->routes[server->routes_count++].callback[method] = callback;
	} else {
		server->routes[route_index].callback[method] = callback;
	}
	
	return 0;
}

static inline
void p(struct http_header* header, char* s) 
{
	if (!header) {
		return;
	}

	p(header->left, s);
	char* copy_s = strdup(s);
	sprintf(s, "%s%s: %s\r\n", 
		copy_s, header->key, header_get(header, header->key));
	free(copy_s);
	p(header->right, s);
}

char* transaction_to_string(struct http_transaction transaction[static 1])
{
	char* trunks = malloc(BUFF_SIZE);
	char* status = header_get(transaction->headers, "Status");
	
	sprintf(trunks, "%s %s %s\r\n",
		HTTP_VERSION, status != NULL ? status : OK, "");
	p(transaction->headers, trunks);

    char *tmp = trunks; 
	asprintf(&trunks, "%s\r\n%s",
		trunks, transaction->entity_body);
    free(tmp);           
	
	return trunks;
}

static inline
void debug_header(struct http_header* header)
{
	if (!header) return;

	debug_header(header->left);
	debug_info("%s %s\n", header->key, header_get(header, header->key));
	debug_header(header->right);
}

char* handle_request(struct http_server server[static 1], const char buffer[static 1])
{	
	struct http_header* request = parse_request(buffer);
	if (request == NULL) {
		debug_error("NOT AN HTTP REQUEST\n");
		return NULL;
	}
	
	int32_t method;
	if (!strcmp(header_get(request, "Method"), "GET")) {
		method = M_GET;
	} else if (!strcmp(header_get(request, "Method"), "POST")) {
		method = M_POST;
	} else if (!strcmp(header_get(request, "Method"), "HEAD")) {
		method = M_HEAD;
	}
	debug("%s %s\n", header_get(request, "Method"), header_get(request, "Request-URI"));
	debug_header(request);

	struct http_transaction transaction = { 0 }; 
	int32_t route_index = get_route(server, header_get(request, "Request-URI"));

	if (route_index >= 0) {
		transaction = server->routes[route_index].callback[method](request);
	} else {
		debug_error("NO ROUTE %s %s\n", header_get(request, "Method"), header_get(request, "Request-URI"));
		header_set(&transaction.headers, "Status", NOT_FOUND); 
		transaction_set_body(&transaction, "<H1>404</H1>");
	} 
		
	char* resp = transaction_to_string(&transaction);
	header_free(request);	
	header_free(transaction.headers);	
	return resp;
}

void handle_poll(struct http_server server[static 1], int32_t socket_server)
{
	char buff[BUFF_SIZE];
	for (;;) {
		poll(c_poll.clients, c_poll.size, -1);
		for (size_t i = 0; i < c_poll.size; ++i) {
			if (c_poll.clients[i].revents & POLLIN) {
				if (c_poll.clients[i].fd == socket_server) {
					push_poll(accept(socket_server, NULL, NULL));
				} else {
					int32_t nbytes = recv(c_poll.clients[i].fd, buff, BUFF_SIZE - 1, 0);
					if (nbytes) {
						buff[nbytes] = '\0';

						char* response = handle_request(server, buff);
						if (response != NULL) {
							send(c_poll.clients[i].fd, response, strlen(response), 0);
							free(response);
						} else {
							send(c_poll.clients[i].fd, "NO HTTP REQ", strlen("NO HTTP REQ"), 0);
						}	

					} else {
						close(c_poll.clients[i].fd);
						c_poll.clients[i] = c_poll.clients[--c_poll.size];
					}
				}
			}
		}
	}
}

#define E_GETADDRINFO -1
#define E_BIND        -2
#define E_LISTEN      -3

int32_t http_server_listen(struct http_server server[static 1])
{
    debug("getaddrinfo...\n");

	struct addrinfo* srv_info = { 0 };
	if (getaddrinfo(NULL, server->port, &(struct addrinfo) {
        	.ai_family   = AF_UNSPEC,
        	.ai_socktype = SOCK_STREAM,
        	.ai_flags    = AI_PASSIVE
    	}, &srv_info))
    {
    	return E_GETADDRINFO;
    }

    int32_t socket_server = socket(srv_info->ai_family, srv_info->ai_socktype, srv_info->ai_protocol);

    debug("bind...\n");
    if (bind(socket_server, srv_info->ai_addr, srv_info->ai_addrlen)) {
    	return E_BIND;
    }

    debug("listen...\n");
    if (listen(socket_server, BACKLOG)) {
    	return E_LISTEN;
    }

	push_poll(socket_server);
	handle_poll(server, socket_server);

	debug("SHUTTING FUCKING DOWN\n");
	close(socket_server);
    freeaddrinfo(srv_info);	

	return EXIT_SUCCESS;
}
