#ifndef DEBUG_H
#define DEBUG_H

#include <stdio.h>

#define DEBUG

#ifdef DEBUG
#define debug(...) printf("\e[0;32m[DEBUG]\e[0m "  __VA_ARGS__)
#define debug_info(...) printf("\e[0;93m[INFO!]\e[0m "  __VA_ARGS__)
#define debug_error(...) printf("\e[0;91m[ERROR]\e[0m "  __VA_ARGS__)
#else 
#define debug(fmt, ...)
#define debug_info(fmt, ...)
#define debug_error(fmt, ...)
#endif

#endif // DEBUG_H
