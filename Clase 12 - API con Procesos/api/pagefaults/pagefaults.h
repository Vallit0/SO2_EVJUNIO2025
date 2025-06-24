#ifndef PAGEFAULTS_H
#define PAGEFAULTS_H

#include <microhttpd.h>

// Declaración de la función que maneja el endpoint "/pagef"
int handle_pagefaults_request(struct MHD_Connection *connection);

#endif
