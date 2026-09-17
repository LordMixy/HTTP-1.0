#include "../include/http_header.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define E_INIT_FAILED 1

[[nodiscard]] static inline
struct http_header* header_init(char key[static 1], char value[static 1])
{
    struct http_header* header;
    if ((header = malloc(sizeof(struct http_header)))) {
		snprintf(header->key, 100, "%s", key);
		snprintf(header->value, 50, "%s", value);
		header->right = header->left = NULL;
    }
    return header;
}

[[nodiscard]] static inline
struct http_header** header_find_indirect(struct http_header* header[static 1], char key[static 1])
{
    while (*header && strcmp((*header)->key, key)) {
        if (strcmp(key, (*header)->key) > 0) {
            header = &(*header)->right;
        } else if (strcmp(key, (*header)->key) < 0) {
            header = &(*header)->left;	
        }
    }
    return header;
}

[[nodiscard]]
char* header_get(struct http_header header[static 1], char key[static 1])
{
    struct http_header* entry = *header_find_indirect(&header, key); 
	return entry ? entry->value : NULL;
}

int32_t header_set(struct http_header* header[static 1], char key[static 1], char value[static 1])
{
	struct http_header** indirect = header_find_indirect(header, key);	
    
    if (*indirect) {
    	snprintf((*indirect)->value, 100, "%s", value);
    } else if (!(*indirect = header_init(key, value))) {
        return E_INIT_FAILED;
    }
    
	return 0;
}

void header_free(struct http_header* header)
{
	if (!header) {
		return;
	}
	
	struct http_header* left = header->left;
	struct http_header* right = header->right;

	free(header);
	header_free(left);
	header_free(right);
}
