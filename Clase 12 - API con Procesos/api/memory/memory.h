#ifndef MEMORY_H
#define MEMORY_H

#include <microhttpd.h>

// Declaración de la función que maneja el endpoint "/memory"
int handle_memory_request(struct MHD_Connection *connection);

#endif
