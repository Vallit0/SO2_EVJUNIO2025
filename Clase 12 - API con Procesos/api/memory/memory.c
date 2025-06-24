#include <sys/syscall.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <cjson/cJSON.h>
#include <microhttpd.h>
#include "memory.h"

// Estructura para la información de memoria
struct mem_info {
    unsigned long total_memory;
    unsigned long free_memory;
    unsigned long used_memory;
    unsigned long cached_memory;
};

// Función para convertir la estructura a JSON
char* mem_info_to_json(struct mem_info* info) {
    cJSON *json = cJSON_CreateObject();

    cJSON_AddNumberToObject(json, "total_memory", info->total_memory);
    cJSON_AddNumberToObject(json, "free_memory", info->free_memory);
    cJSON_AddNumberToObject(json, "used_memory", info->used_memory);
    cJSON_AddNumberToObject(json, "cached_memory", info->cached_memory);

    char *json_string = cJSON_Print(json);
    cJSON_Delete(json); // liberar la memoria del JSON

    return json_string; // retornar el string JSON
}

// Función que maneja el endpoint "/memory"
int handle_memory_request(struct MHD_Connection *connection) {
    struct mem_info info;

    // Realizar la syscall para obtener la información de memoria
    if (syscall(339, &info) != 0) {
        const char *error_str = "{\"error\": \"sys_meminfo syscall failed\"}";
        struct MHD_Response *response = MHD_create_response_from_buffer(strlen(error_str),
                                                                        (void *)error_str, MHD_RESPMEM_PERSISTENT);
        add_cors_headers(response);
        MHD_queue_response(connection, MHD_HTTP_INTERNAL_SERVER_ERROR, response);
        MHD_destroy_response(response);
        return MHD_HTTP_INTERNAL_SERVER_ERROR;
    }

    // Convertir la estructura a JSON
    char *json_response = mem_info_to_json(&info);

    // Crear respuesta HTTP con el JSON
    struct MHD_Response *response = MHD_create_response_from_buffer(strlen(json_response),
                                                                    (void *)json_response, MHD_RESPMEM_MUST_FREE);
    
    add_cors_headers(response);

    int ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
    MHD_destroy_response(response);

    return ret;
}
