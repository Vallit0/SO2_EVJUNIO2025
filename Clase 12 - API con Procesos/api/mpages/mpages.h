#ifndef MPAGES_H
#define MPAGES_H

#include <microhttpd.h>

// Declaración de la función que maneja el endpoint "/memory_pages"
int handle_memory_pages_request(struct MHD_Connection *connection);

#endif
