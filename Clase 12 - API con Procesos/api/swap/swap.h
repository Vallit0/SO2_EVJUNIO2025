#ifndef SWAP_H
#define SWAP_H

#include <microhttpd.h>

// Declaración de la función que maneja el endpoint "/swap"
int handle_swap_request(struct MHD_Connection *connection);

#endif
