#include <sys/syscall.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <cjson/cJSON.h>
#include <microhttpd.h>
#include "mpages.h"


// Estructura para la información de las páginas de memoria
struct memory_pages_info {
    unsigned long total;
    unsigned long active_pages;
    unsigned long inactive_pages;
    unsigned long active_pages_nr;
    unsigned long inactive_pages_nr;
    unsigned long active_pages_mem;
    unsigned long inactive_pages_mem;
};

// Función para convertir la estructura a JSON
char* memory_pages_info_to_json(struct memory_pages_info* info) {
    cJSON *json = cJSON_CreateObject();

    cJSON_AddNumberToObject(json, "total", info->total);
    cJSON_AddNumberToObject(json, "active_pages", info->active_pages);
    cJSON_AddNumberToObject(json, "inactive_pages", info->inactive_pages);
    cJSON_AddNumberToObject(json, "active_pages_nr", info->active_pages_nr);
    cJSON_AddNumberToObject(json, "inactive_pages_nr", info->inactive_pages_nr);
    cJSON_AddNumberToObject(json, "active_pages_mem", info->active_pages_mem);
    cJSON_AddNumberToObject(json, "inactive_pages_mem", info->inactive_pages_mem);

    char *json_string = cJSON_Print(json);
    cJSON_Delete(json); // liberar la memoria del JSON

    return json_string; // retornar el string JSON
}

// Función que maneja el endpoint "/memory_pages"
int handle_memory_pages_request(struct MHD_Connection *connection) {
    struct memory_pages_info info;

    // Realizar la syscall para obtener la información de las páginas de memoria
    if (syscall(342, &info) != 0) {
        const char *error_str = "{\"error\": \"sys_memorypages syscall failed\"}";
        struct MHD_Response *response = MHD_create_response_from_buffer(strlen(error_str),
                                                                        (void *)error_str, MHD_RESPMEM_PERSISTENT);
        add_cors_headers(response);
        MHD_queue_response(connection, MHD_HTTP_INTERNAL_SERVER_ERROR, response);
        MHD_destroy_response(response);
        return MHD_HTTP_INTERNAL_SERVER_ERROR;
    }

    // Convertir la estructura a JSON
    char *json_response = memory_pages_info_to_json(&info);

    // Crear respuesta HTTP con el JSON
    struct MHD_Response *response = MHD_create_response_from_buffer(strlen(json_response),
                                                                    (void *)json_response, MHD_RESPMEM_MUST_FREE);
    
    add_cors_headers(response);
    
    int ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
    MHD_destroy_response(response);

    return ret;
}
