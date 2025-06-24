#include <sys/syscall.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <cjson/cJSON.h>
#include <microhttpd.h>
#include "pagefaults.h"

// Estructura para los page faults
struct page_faults_info {
    unsigned long minorf_nd;
    unsigned long majorf_nd;
    unsigned long minorf_tk;
    unsigned long majorf_tk;
    unsigned long minorf_vms;
    unsigned long majorf_vms;
};

// Función para convertir la estructura a JSON
char* struct_to_json(struct page_faults_info* info) {
    cJSON *json = cJSON_CreateObject();

    cJSON_AddNumberToObject(json, "minorf_nd", info->minorf_nd);
    cJSON_AddNumberToObject(json, "majorf_nd", info->majorf_nd);
    cJSON_AddNumberToObject(json, "minorf_tk", info->minorf_tk);
    cJSON_AddNumberToObject(json, "majorf_tk", info->majorf_tk);
    cJSON_AddNumberToObject(json, "minorf_vms", info->minorf_vms);
    cJSON_AddNumberToObject(json, "majorf_vms", info->majorf_vms);

    char *json_string = cJSON_Print(json);
    cJSON_Delete(json); // liberar la memoria del JSON

    return json_string; // retornar el string JSON
}

// Función que maneja el endpoint "/pagef"
int handle_pagefaults_request(struct MHD_Connection *connection) {
    struct page_faults_info info;

    // Realizar la syscall para obtener los page faults
    if (syscall(341, &info) != 0) {
        const char *error_str = "{\"error\": \"sys_pagefaults syscall failed\"}";
        struct MHD_Response *response = MHD_create_response_from_buffer(strlen(error_str),
                                                                        (void *)error_str, MHD_RESPMEM_PERSISTENT);
        add_cors_headers(response);
        MHD_queue_response(connection, MHD_HTTP_INTERNAL_SERVER_ERROR, response);
        MHD_destroy_response(response);
        return MHD_HTTP_INTERNAL_SERVER_ERROR;
    }

    // Convertir la estructura a JSON
    char *json_response = struct_to_json(&info);

    // Crear respuesta HTTP con el JSON
    struct MHD_Response *response = MHD_create_response_from_buffer(strlen(json_response),
                                                                    (void *)json_response, MHD_RESPMEM_MUST_FREE);
    
    add_cors_headers(response);
    
    int ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
    MHD_destroy_response(response);

    return ret;
}
