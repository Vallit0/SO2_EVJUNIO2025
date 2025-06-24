#ifndef CORS_H
#define CORS_H

#include <microhttpd.h>

// Declaración de la función que maneja el endpoint "/swap"
void add_cors_headers(struct MHD_Response *response);

#endif