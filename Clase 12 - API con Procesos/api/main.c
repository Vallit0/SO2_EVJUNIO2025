#include <microhttpd.h>
#include <stdio.h>
#include <string.h>
#include "pagefaults/pagefaults.h"
#include "memory/memory.h"
#include "swap/swap.h"
#include "mpages/mpages.h"
#include "memory_processes/mprocesses.h"
#include "cors/cors.h"

#define PORT 8000

// Función que maneja las solicitudes HTTP y delega a los handlers según la URL
int handle_request(void *cls, struct MHD_Connection *connection,
                   const char *url, const char *method,
                   const char *version, const char *upload_data,
                   size_t *upload_data_size, void **con_cls) {


    struct MHD_Response *response;
    // Si es una solicitud OPTIONS, manejarla como parte del preflight de CORS
    if (strcmp(method, "OPTIONS") == 0) {
        response = MHD_create_response_from_buffer(0, "", MHD_RESPMEM_PERSISTENT);
        add_cors_headers(response);
        int ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
        MHD_destroy_response(response);
        return ret;
    }

    if (strcmp(url, "/pagef") == 0) {
        return handle_pagefaults_request(connection);
    } else if (strcmp(url, "/memory") == 0) {
        return handle_memory_request(connection);
    } else if (strcmp(url, "/swap") == 0) {
        return handle_swap_request(connection);
    } else if (strcmp(url, "/memory_pages") == 0) {
        return handle_memory_pages_request(connection);
    } else if (strcmp(url, "/memory_processes") == 0) {
        return handle_top_processes_request(connection);
    }

    const char *error_str = "{\"error\": \"Endpoint not found\"}";
    response = MHD_create_response_from_buffer(strlen(error_str),
                                                                    (void *)error_str, MHD_RESPMEM_PERSISTENT);

    // Agregar encabezados CORS
    add_cors_headers(response);

    int ret = MHD_queue_response(connection, MHD_HTTP_NOT_FOUND, response);
    MHD_destroy_response(response);
    return ret;
}

int main() {
    struct MHD_Daemon *daemon;

    // Iniciar el servidor HTTP en el puerto 8888
    daemon = MHD_start_daemon(MHD_USE_SELECT_INTERNALLY, PORT, NULL, NULL,
                              &handle_request, NULL, MHD_OPTION_END);
    if (NULL == daemon) return 1;

    printf("Servidor API corriendo en http://localhost:%d\n", PORT);

    // Mantener el servidor corriendo
    getchar();

    // Detener el servidor cuando se cierre
    MHD_stop_daemon(daemon);
    return 0;
}
