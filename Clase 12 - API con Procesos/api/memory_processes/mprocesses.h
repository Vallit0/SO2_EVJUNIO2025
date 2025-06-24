#ifndef MEMORY_PROCESSES_H
#define MEMORY_PROCESSES_H

#include <microhttpd.h>

// Declaración de la función que maneja el endpoint "/top_processes"
int handle_top_processes_request(struct MHD_Connection *connection);

#endif
