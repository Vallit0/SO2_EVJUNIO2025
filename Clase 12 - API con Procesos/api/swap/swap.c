#include <sys/syscall.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <cjson/cJSON.h>
#include <microhttpd.h>
#include "swap.h"

// Estructura para la información de swap
struct swap_info {
    unsigned long total_si;
    unsigned long free_si;
    unsigned long used_si;
    unsigned long total_sp;
    unsigned long free_sp;
    unsigned long used_sp;
    unsigned long total_mem;
    unsigned long free_mem;
    unsigned long used_mem;
};

// Función para convertir la estructura a JSON
char* swap_info_to_json(struct swap_info* info) {
    cJSON *json = cJSON_CreateObject();

    cJSON_AddNumberToObject(json, "total_si", info->total_si);
    cJSON_AddNumberToObject(json, "free_si", info->free_si);
    cJSON_AddNumberToObject(json, "used_si", info->used_si);
    cJSON_AddNumberToObject(json, "total_sp", info->total_sp);
    cJSON_AddNumberToObject(json, "free_sp", info->free_sp);
    cJSON_AddNumberToObject(json, "used_sp", info->used_sp);
    cJSON_AddNumberToObject(json, "total_mem", info->total_mem);
    cJSON_AddNumberToObject(json, "free_mem", info->free_mem);
    cJSON_AddNumberToObject(json, "used_mem", info->used_mem);

    char *json_string = cJSON_Print(json);
    cJSON_Delete(json); // liberar la memoria del JSON

    return json_string; // retornar el string JSON
}

// Función que maneja el endpoint "/swap"
int handle_swap_request(struct MHD_Connection *connection) {
    struct swap_info info;

    // Realizar la syscall para obtener la información de swap
    if (syscall(340, &info) != 0) {
        const char *error_str = "{\"error\": \"sys_swapinfo syscall failed\"}";
        struct MHD_Response *response = MHD_create_response_from_buffer(strlen(error_str),
                                                                        (void *)error_str, MHD_RESPMEM_PERSISTENT);
        add_cors_headers(response);
        MHD_queue_response(connection, MHD_HTTP_INTERNAL_SERVER_ERROR, response);
        MHD_destroy_response(response);
        return MHD_HTTP_INTERNAL_SERVER_ERROR;
    }

    // Convertir la estructura a JSON
    char *json_response = swap_info_to_json(&info);

    // Crear respuesta HTTP con el JSON
    struct MHD_Response *response = MHD_create_response_from_buffer(strlen(json_response),
                                                                    (void *)json_response, MHD_RESPMEM_MUST_FREE);
    
    // Agregar encabezados CORS a la respuesta
    add_cors_headers(response);
    
    int ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
    MHD_destroy_response(response);

    return ret;
}
