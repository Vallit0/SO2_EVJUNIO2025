#include <sys/syscall.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <cjson/cJSON.h>
#include <microhttpd.h>
#include "mprocesses.h"

// Estructura para la información de los procesos
struct process_info {
    pid_t pid;
    unsigned long memory;
};

// Función para convertir la estructura a JSON
char* process_info_to_json(struct process_info* procs, size_t count) {
    cJSON *json = cJSON_CreateArray();

    for (size_t i = 0; i < count; i++) {
        cJSON *proc_json = cJSON_CreateObject();
        cJSON_AddNumberToObject(proc_json, "pid", procs[i].pid);
        cJSON_AddNumberToObject(proc_json, "memory", procs[i].memory);
        cJSON_AddItemToArray(json, proc_json);
    }

    char *json_string = cJSON_Print(json);
    cJSON_Delete(json); // liberar la memoria del JSON

    return json_string; // retornar el string JSON
}

// Función que maneja el endpoint "/top_processes"
int handle_top_processes_request(struct MHD_Connection *connection) {
    struct process_info top_procs[5];

    // Llamar a la syscall para obtener los procesos con más uso de memoria
    if (syscall(342, top_procs) == -1) {
        const char *error_str = "{\"error\": \"syscall failed\"}";
        struct MHD_Response *response = MHD_create_response_from_buffer(strlen(error_str),
                                                                        (void *)error_str, MHD_RESPMEM_PERSISTENT);
        add_cors_headers(response);
        MHD_queue_response(connection, MHD_HTTP_INTERNAL_SERVER_ERROR, response);
        MHD_destroy_response(response);
        return MHD_HTTP_INTERNAL_SERVER_ERROR;
    }

    // Convertir la estructura a JSON
    char *json_response = process_info_to_json(top_procs, 5);

    // Crear respuesta HTTP con el JSON
    struct MHD_Response *response = MHD_create_response_from_buffer(strlen(json_response),
                                                                    (void *)json_response, MHD_RESPMEM_MUST_FREE);
    
    add_cors_headers(response);
    
    int ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
    MHD_destroy_response(response);

    return ret;
}
