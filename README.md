
# Proyecto: USAC Linux

# Syscalls

## Crear Syscalls

Creamos una carpeta dentro del directorio principal del kernel. El nombre de esta carpeta será `syscalls_project`.  

Dentro de esa carpeta, vamos a crear un archivo `sycalls_project.c`

Dentro de nuestro archivo `custom_syscalls.c` vamos a escribir las syscalls que se detallan a continuación.

Includes y definiciones a utilizar:
```c
#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/sched.h>
#include <linux/pid.h>
#include <linux/mm.h>
#include <linux/swap.h>
#include <linux/vmstat.h>
#include <linux/init.h>
#include <linux/fs.h>

#define MEMINFO_BUFFER_SIZE 4096
#define VMSTAT_BUFFER_SIZE 4096
```

### Syscall: Uso de memoria
Esta syscall muestra la memoria libre, utilizada y en caché.

La syscall `memory_info` proporciona información sobre el estado actual de la memoria del sistema, copiando esta información en una estructura `mem_info` proporcionada por el usuario. La syscall obtiene los datos de memoria del sistema, como la memoria total, libre, utilizada y en caché, y los devuelve en kilobytes. Luego, estos datos se copian a la estructura en el espacio de usuario. Si la operación de copia falla, la syscall devuelve un error `(-EFAULT)`.

Resumen del proceso:
Obtiene la información de memoria del sistema.
Calcula memoria total, libre, utilizada y en caché.
Copia esta información al espacio de usuario.

```c
// Definición de estructura para almacenar la información de memoria
struct mem_info {
    unsigned long total_memory;
    unsigned long free_memory;
    unsigned long used_memory;
    unsigned long cached_memory;
};

SYSCALL_DEFINE1(memory_info, struct mem_info __user *, info) {
    struct sysinfo si;
    struct mem_info kernel_info;

    // Obtener la información del sistema
    si_meminfo(&si);

    // Calcular memoria total, libre, utilizada y en caché (en KB)
    kernel_info.total_memory = si.totalram << (PAGE_SHIFT - 10);    
    kernel_info.free_memory = si.freeram << (PAGE_SHIFT - 10);     
    kernel_info.cached_memory = si.bufferram << (PAGE_SHIFT - 10);
    kernel_info.used_memory = kernel_info.total_memory - kernel_info.free_memory;

    // Copiar la estructura al espacio de usuario
    if (copy_to_user(info, &kernel_info, sizeof(struct mem_info)))
        return -EFAULT;

    return 0;
}
```

### Syscall: Páginas de memoria de swap usadas y libres
Esta llamada al sistema recupera el número de páginas de swap libres y utilizadas. 

La syscall `swapinfo` recopila información detallada sobre el estado de la memoria y el espacio de intercambio (swap) del sistema, utilizando tres métodos distintos, y luego copia estos datos a una estructura `swap_info` en el espacio de usuario.

Explicación de los tres métodos:

Método 1: Utiliza la función `si_meminfo()` para obtener la información sobre el total de páginas de swap y las páginas libres. Calcula el swap usado restando las páginas libres de las totales.

Método 2: Directamente utiliza variables globales del kernel (`total_swap_pages` y `get_nr_swap_pages()`) para obtener el número total de páginas de swap y las páginas libres, y calcula el swap usado.

Método 3: Abre y lee el archivo `/proc/meminfo` para extraer las líneas que contienen `SwapTotal` y `SwapFree`, obteniendo la cantidad total y libre de swap en kilobytes, y calcula el swap usado.

Finalmente, la información de swap y memoria obtenida por los tres métodos se copia al espacio de usuario. Si la copia falla, la syscall devuelve un error `(-EFAULT)`.

```c
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

SYSCALL_DEFINE1(swapinfo, struct swap_info __user *, info) {
    struct sysinfo si;
    struct swap_info kernel_info;

    // ----- Metodo 1 -----

    // Obtener la información de swap del sistema
    si_meminfo(&si);

    // Calcular el número total de páginas de swap
    kernel_info.total_si = si.totalswap;
    kernel_info.free_si = si.freeswap;
    kernel_info.used_si = kernel_info.total_si - kernel_info.free_si;

    // ----- Metodo 2 -----

    kernel_info.total_sp = total_swap_pages;
    kernel_info.free_sp = get_nr_swap_pages();
    kernel_info.used_sp = kernel_info.total_sp - kernel_info.free_sp;

    // ----- Método 3 -----
    struct file *f;
    loff_t pos = 0;
    char *buf;
    char *total_line, *free_line;
    unsigned long total_value = 0, free_value = 0;
    int bytes_read;

    // Reservamos memoria para el buffer
    buf = kmalloc(MEMINFO_BUFFER_SIZE, GFP_KERNEL);
    if (!buf)
        return -ENOMEM;

    // Abrimos el archivo /proc/meminfo
    f = filp_open("/proc/meminfo", O_RDONLY, 0);
    if (IS_ERR(f)) {
        kfree(buf);
        return PTR_ERR(f);
    }

    // Leemos el archivo
    bytes_read = kernel_read(f, buf, MEMINFO_BUFFER_SIZE, &pos);
    if (bytes_read < 0) {
        filp_close(f, NULL);
        kfree(buf);
        return bytes_read;
    }

    // Cerrar el archivo
    filp_close(f, NULL);

    // Encontramos la línea de "Active:"
    total_line = strstr(buf, "SwapTotal:");
    if (total_line)
        sscanf(total_line, "SwapTotal: %lu kB", &total_value);

    // Encontramos la línea de "Inactive:"
    free_line = strstr(buf, "SwapFree:");
    if (free_line)
        sscanf(free_line, "SwapFree: %lu kB", &free_value);

    kernel_info.total_mem = total_value;
    kernel_info.free_mem = free_value;
    kernel_info.used_mem = total_value - free_value;

    kfree(buf);

    // Copiar la estructura al espacio de usuario
    if (copy_to_user(info, &kernel_info, sizeof(struct swap_info)))
        return -EFAULT;

    return 0;
}
```

### Syscall: Cantidad de fallos de página
Esta llamada al sistema devuelve el número total de fallos de página, divididos en fallos de página menores y mayores desde el inicio del sistema.


La syscall `pagefaults` recopila información sobre los fallos de página (páginas de memoria solicitadas por el sistema pero no encontradas en la memoria física) a nivel global y de proceso, usando tres métodos diferentes. Los datos se copian a una estructura `page_faults_info` en el espacio de usuario.

Descripción de los métodos:

Método 1: Utiliza la función `global_node_page_state()` para obtener el número total de fallos de página menores (`minorf_nd`) y mayores (`majorf_nd`) desde el inicio del sistema.

Método 2: Extrae los fallos de página menores (`minorf_tk`) y mayores (`majorf_tk`) del proceso actual (la estructura task_struct del proceso que llama a la syscall).

Método 3: Lee el archivo `/proc/vmstat` para extraer las métricas de fallos de página menores (`pgfault`) y mayores (`pgmajfault`), obteniendo estos valores a nivel de sistema.

Finalmente, la información de fallos de página obtenida se copia al espacio de usuario. Si la copia falla, la syscall devuelve un error `(-EFAULT)`.








```c

// Definición de estructura para almacenar los fallos de página
struct page_faults_info {
    unsigned long minorf_nd;
    unsigned long majorf_nd;
    
    unsigned long minorf_tk;
    unsigned long majorf_tk;

    unsigned long minorf_vms;
    unsigned long majorf_vms;
};

SYSCALL_DEFINE1(pagefaults, struct page_faults_info __user *, info) {
    struct page_faults_info kernel_info;

    // Obtener el número de fallos de página menores y mayores desde el inicio del sistema
    // Metodo 1
    kernel_info.minorf_nd = global_node_page_state(PGFAULT);
    kernel_info.majorf_nd = global_node_page_state(PGMAJFAULT);

    // Metodo 2
    struct task_struct *task = current;
    kernel_info.minorf_tk = task->min_flt;  
    kernel_info.majorf_tk = task->maj_flt;
    
    // Metodo 3
    struct file *f;
    loff_t pos = 0;
    char *buf;
    char *pgfault_line, *pgmajfault_line;
    unsigned long pgfault_value = 0, pgmajfault_value = 0;
    int bytes_read;

    buf = kmalloc(VMSTAT_BUFFER_SIZE, GFP_KERNEL);
    if (!buf)
        return -ENOMEM;

    f = filp_open("/proc/vmstat", O_RDONLY, 0);
    if (IS_ERR(f)) {
        kfree(buf);
        return PTR_ERR(f);
    }

    bytes_read = kernel_read(f, buf, MEMINFO_BUFFER_SIZE, &pos);
    if (bytes_read < 0) {
        filp_close(f, NULL);
        kfree(buf);
        return bytes_read;
    }

    filp_close(f, NULL);

    pgfault_line = strstr(buf, "pgfault");
    if (pgfault_line)
        sscanf(pgfault_line, "pgfault %lu", &pgfault_value);

    pgmajfault_line = strstr(buf, "pgmajfault");
    if (pgmajfault_line)
        sscanf(pgmajfault_line, "pgmajfault %lu", &pgmajfault_value);

    kernel_info.minorf_vms = pgfault_value;
    kernel_info.majorf_vms = pgmajfault_value;

    kfree(buf);

    if (copy_to_user(info, &kernel_info, sizeof(struct page_faults_info)))
        return -EFAULT;

    return 0;
}

```

### Syscall: Páginas de memoria activas e inactivas
Esta llamada al sistema proporciona el número de páginas de memoria activas e inactivas en el sistema

La syscall `memorypages` obtiene información sobre las páginas de memoria activas e inactivas del sistema, utilizando tres métodos diferentes. Estos datos se copian a una estructura `memory_pages_info` en el espacio de usuario.

Descripción de los métodos:

Método 1: Usa la función `si_meminfo()` para obtener la memoria total y las páginas inactivas (libres), y calcula las páginas activas restando las páginas inactivas de las totales.

Método 2: Utiliza el estado global de las páginas del sistema, mediante las funciones global`_node_page_state()`, para obtener el número total de páginas activas e inactivas (de archivos y anónimas).

Método 3: Lee el archivo `/proc/meminfo` para extraer las líneas que contienen las métricas de "Active" e "Inactive", que representan la memoria activa e inactiva en kilobytes.

Finalmente, la información obtenida se copia al espacio de usuario. Si la copia falla, la syscall retorna un error `(-EFAULT`).

```c

// Definición de estructura para almacenar el conteo de páginas activas e inactivas
struct memory_pages_info {
    unsigned long total;
    unsigned long active_pages;
    unsigned long inactive_pages;

    unsigned long active_pages_nr;
    unsigned long inactive_pages_nr;

    unsigned long active_pages_mem;
    unsigned long inactive_pages_mem;
};

SYSCALL_DEFINE1(memorypages, struct memory_pages_info __user *, info) {
    struct memory_pages_info kernel_info;
    struct sysinfo si;
    si_meminfo(&si);
    
    // Método 1
    kernel_info.total = si.totalram;
    kernel_info.inactive_pages = si.freeram;
    kernel_info.active_pages = kernel_info.total - kernel_info.inactive_pages;

    // Metodo 2
    kernel_info.active_pages_nr = global_node_page_state(NR_ACTIVE_FILE) + global_node_page_state(NR_ACTIVE_ANON);
    kernel_info.inactive_pages_nr = global_node_page_state(NR_INACTIVE_FILE) + global_node_page_state(NR_INACTIVE_ANON);
    
    // Metodo 3
    struct file *f;
    loff_t pos = 0;
    char *buf;
    char *active_line, *inactive_line;
    unsigned long active_value = 0, inactive_value = 0;
    int bytes_read;
    
    // Reservamos memoria para el buffer
    buf = kmalloc(MEMINFO_BUFFER_SIZE, GFP_KERNEL);
    if (!buf)
        return -ENOMEM;

    // Abrimos el archivo /proc/meminfo
    f = filp_open("/proc/meminfo", O_RDONLY, 0);
    if (IS_ERR(f)) {
        kfree(buf);
        return PTR_ERR(f);
    }

    // Leemos el archivo
    bytes_read = kernel_read(f, buf, MEMINFO_BUFFER_SIZE, &pos);
    if (bytes_read < 0) {
        filp_close(f, NULL);
        kfree(buf);
        return bytes_read;
    }

    // Cerrar el archivo
    filp_close(f, NULL);

    // Encontramos la línea de "Active:"
    active_line = strstr(buf, "Active:");
    if (active_line)
        sscanf(active_line, "Active: %lu kB", &active_value);

    // Encontramos la línea de "Inactive:"
    inactive_line = strstr(buf, "Inactive:");
    if (inactive_line)
        sscanf(inactive_line, "Inactive: %lu kB", &inactive_value);

    kernel_info.active_pages_mem = active_value;
    kernel_info.inactive_pages_mem = inactive_value;

    kfree(buf);

    // Copiar la estructura al espacio de usuario
    if (copy_to_user(info, &kernel_info, sizeof(struct memory_pages_info)))
        return -EFAULT;

    return 0;
}

```

### Syscall: Procesos que más memoria utilizan
Esta llamada al sistema recupera información sobre los 5 procesos que consumen más memoria en el sistema. 

La syscall `top_memory_processes` obtiene los 5 procesos que están utilizando más memoria en el sistema y copia esa información a una estructura `process_info` proporcionada por el usuario.

Funcionamiento:

1. La syscall itera sobre todos los procesos activos en el sistema utilizando `for_each_process()`.
2. Para cada proceso, verifica si tiene un espacio de direcciones válido (`task->mm`) y obtiene el uso de memoria virtual (RSS - Resident Set Size), convirtiendo el valor de páginas de memoria a bytes.
3. Compara el uso de memoria del proceso actual con los procesos que ya están en el "top 5" de mayor uso de memoria. Si es mayor, actualiza la lista de los 5 procesos, desplazando los elementos hacia abajo y agregando el proceso nuevo en la posición correspondiente.
4. El proceso no guarda el nombre del proceso, aunque hay una línea comentada para hacer esto.
5. Finalmente, la lista de los 5 procesos con más memoria utilizada se copia al espacio de usuario. Si la copia falla, la syscall devuelve un error `(-EFAULT)`.
Esta syscall permite al usuario consultar los procesos con mayor consumo de memoria en tiempo real.

```c

// Definición de estructura para almacenar los procesos
struct process_info {
    pid_t pid;
    unsigned long memory;
    // char name[TASK_COMM_LEN];
};

SYSCALL_DEFINE1(top_memory_processes, struct process_info __user *, top_procs) {
    struct task_struct *task;
    struct process_info top5[5] = {{0}};
    
    // Iterar sobre todos los procesos en el sistema
    rcu_read_lock();
    for_each_process(task) {
        unsigned long mem_usage = 0;
        
        if (task->mm) {
            // Obtener la memoria virtual (resident set size)
            // Convertir páginas a bytes
            mem_usage = get_mm_rss(task->mm) << PAGE_SHIFT; 
        
            // Si es mayor que los que están en el top 5, actualizar la lista
            for (int i = 0; i < 5; i++) {
                if (mem_usage > top5[i].memory) {
                    // Mover hacia abajo los valores en la lista
                    for (int j = 4; j > i; j--) {
                        top5[j] = top5[j-1];
                    }
                    
                    // Insertar el nuevo proceso
                    top5[i].pid = task->pid;
                    top5[i].memory = mem_usage;

                    // Obtener el nombre del proceso
                    // strncpy(top5[i].name, task->comm, TASK_COMM_LEN);
                    break;
                }
            }
        }
    }
    rcu_read_unlock();


    // Copiar el resultado al espacio de usuario
    if (copy_to_user(top_procs, top5, sizeof(top5))) {
        return -EFAULT;
    }

    return 0;
}
```

## Crear Makefile

Vamos a crear un archivo `Makefile` en el mismo directorio de syscalls_project.
A este archivo (de nombre 'Makefile'), le agregaremos la siguiente linea:

```Makefile
obj-y := syscalls_project.o
```
Esto se encarga de que el archivo syscalls_project.c sea compilado e incluido en el código fuente del kernel.

## Agregar 'syscalls_project/' al Makefile del kernel

Vamos a ir al directorio principal del kernel y buscaremos el Makefile.

Lo abrimos y en el archivo debemos buscar 'core-y'.

Una vez que lo hayamos encontrado debemos agregar /syscalls_project. De esta forma:

```Makefile
# { Resto del código } 

ifeq ($(KBUILD_EXTMOD),)
# Objects we will link into vmlinux / subdirs we need to visit
core-y		:= custom_syscalls/ syscalls_usac/ syscalls_project/
drivers-y	:=
libs-y		:= lib/
endif # KBUILD_EXTMOD

# { Resto del código }
```

## Agregar syscalls a la tabla de syscalls


Vamos a ir al siguiente directorio: `arch/x86/entry/syscalls`

Y vamos a editar el archivo `syscall_64.tbl`

Luego de desplazarnos en el documento, encontraremos un comentario que nos indica donde podemos agregar nuevas syscalls. Vamos a agregar la nuestra utilizando un número que no esté en uso:

```
339 common  memory_info     sys_memory_info
340 common  swapinfo        sys_swapinfo
341 common  pagefaults      sys_pagefaults
342 common  memorypages     sys_memorypages
343 common  top_memory_processes    sys_top_memory_processes
```

## Agregar syscalls al archivo header de syscalls

Vamos a ir al siguiente directorio `include/linux/`.
Creamos un archivo llamado `syscalls_project.h` con el siguiente contenido y lo guardamos.

```c
#ifndef _SYSCALLS_PROJECT_H
#define _SYSCALLS_PROJECT_H

#include <linux/kernel.h>
#include <linux/syscalls.h>

asmlinkage long sys_memory_info(struct mem_info __user *info);
asmlinkage long sys_swapinfo(struct swap_info __user *info);
asmlinkage long sys_pagefaults(struct page_faults_info __user *info);
asmlinkage long sys_memorypages(struct memory_pages_info __user *info);
asmlinkage long sys_top_memory_processes(struct process_info __user *top_procs);

#endif
```

# Compilar kernel


## 1. Copiar el archivo config
---

Copiaremos el archivo `.config` de nuestro sistema actual a la carpeta principal donde está el kernel.
Por ejemplo:

```bash
cp -v /boot/config-$(uname -r) .config
```

## 2. Modificar archivo de configuración
---

En la carpeta principal en donde se encuentra el kernel, debemos ejecutar el siguiente comando:

```bash
make localmodconfig 
```

En el proceso, apareceran varias preguntas. En este punto, solo se debe presionar `ENTER` para todo.


## 3. Excluir llaves privadas
---

Volver a compilar las llaves privadas de nuestro sistema actual terminará en error. Para evitarlo, se debe ejecutar los siguientes comando:

```bash
# 1
scripts/config --disable SYSTEM_TRUSTED_KEYS
# 2
scripts/config --disable SYSTEM_REVOCATION_KEYS
# 3 Reiniciar las llaves previamente instaladas
scripts/config --set-str CONFIG_SYSTEM_TRUSTED_KEYS ""
# 4 Reiniciar las llaves previamente instaladas
scripts/config --set-str CONFIG_SYSTEM_REVOCATION_KEYS ""
```

## 4. Iniciar compilación
---

Posteriormente, se debe utilizar el siguiente comando para inciar el proceso de compilación:

```bash
fakeroot make
```

Para especificar el número de nucleos que queremos utilizar podemos especificarlo mediante un parametro en el comando

```bash
# Por ejemplo, 2 nucleos:
fakeroot make -j2
```

## 5. Validar compilación
---

Al terminar la compilación, podemos ver si el proces terminó exitosamente con el comando:

```bash
echo $?
```

- Si no devuelve nada o devuelve un cero, todo fue exitoso.
- Si devuelve algo más, es un código de error.

## 6. Instalar el nuevo kernel y reiniciar
---

Estando en la carpeta principal del kernel, debemos ejecutar:

```bash
sudo make modules_install
```

```bash
sudo make install
```

Si hace falta podemos usar `grub-customizer` para montar el nuevo kernel. Por defecto debería montarse automaticamente.

Posteriomente a este proceso, reiniciamos con:

```bash
sudo reboot
```

# Desarrollo de API en C

Se desarrollará una API en C para poder obtener los datos con las syscalls a través de enpoints.

Se debe instalar lo siguiente para poder realizar la API en C:

```sh
sudo apt-get install libmicrohttpd-dev
sudo apt-get install libcjson-dev
```

## Estructura de proyecto

### /main.c
Archivo para iniciar el server
### /cors
Archivos para la función de agregar CORS al server
### /memory, /memory_processes/, /mpages, /pagefaults, /swap
En estos archivos se definen las funciones que son definidas para endpoint y también que se encargaran de obtener la información de las syscalls y luego retornarlas en un JSON

## Archivo 'main.c'

En este archivo se iniciara el server donde estará la API. También se definen los endpoints que habrán y se asignan las funciones definidas para cada endpoint.

```c
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

```

## Ejemplo de endpoint: Memory

### 'memory.h'
Archivo de encabezado para poder hacer los includes correspondientes en el programa

```c
#ifndef MEMORY_H
#define MEMORY_H

#include <microhttpd.h>

// Declaración de la función que maneja el endpoint "/memory"
int handle_memory_request(struct MHD_Connection *connection);

#endif
```

### 'memory.c'

Aquí se definen las funciones que funcionaran en conjunto con el endpoint para brindar una respuesta a una request del usuario. Dentro de estas funciones, se define la que se encarga de obtener los datos haciendo una llamada al sistema mediante la syscall correspondiente.

```c
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
```

Para compilar la API, se debe utilizar el siguiente comando.
```sh
gcc -o api_server main.c cors/cors.c pagefaults/pagefaults.c memory/memory.c swap/swap.c mpages/mpages.c memory_processes/mprocesses.c -lmicrohttpd -lcjson
```

# Frontend
Se utilizo React con Vite para realizar un frontend para visualizar los datos obtenidos de las syscalls. Esto lo lleva a cabo realizando requests a la API que se hizo y se encarga de gestionar los endpoints que permiten hacer las llamadas al sistema.

## Capturas del frontend

![0](./docs/imgs/image.png)
![1](./docs/imgs/image-1.png)
![2](./docs/imgs/image-2.png)
![3](./docs/imgs/image-3.png)
![4](./docs/imgs/image-4.png)

# Errores

## Error 1: Error de declaración en cabecera
Este error sucedió debido a una declaración incorrecta de la SYSCALL.
Al realizar una SYSCALL se debe verificar que tanto la implementación como la declaración en el archivo de encabezado coincida.

![error-1](./docs/imgs/error-1.png)

### Solución
Para solucionar este problema, se debe verificar que la implementación de la syscall (en este caso, en el archivo `syscalls_project.c`) coincida con la declaración en el archivo de cabecera (en este caso `/include/linux/syscalls_project.h`).

**Implementación**

```c
SYSCALL_DEFINE1(top_memory_processes, struct process_info __user *, top_procs) {
    // {Código...}
}
```

**Cabecera**

```c
#ifndef _SYSCALLS_PROJECT_H
#define _SYSCALLS_PROJECT_H

#include <linux/kernel.h>
#include <linux/syscalls.h>

// {Resto de declaraciones...}
asmlinkage long sys_top_memory_processes(struct process_info __user *top_procs);
#endif
```
## Error 2: Error en la compilación
En este error sucedía que la compilación no avanzaba después de cierto punto. Es decir, se quedaba congelado y no avanzaba más. 

![error-2](./docs/imgs/error-2.png)

### Solución
Para corregir este error, se ejecutaron algunos comandos otra vez, ya que se estaba intentando utilizar unicamente `fakeroot make -j2`.
Los comandos a ejecutar para solucionar son los siguientes:

Copiaremos el archivo `.config` de nuestro sistema actual a la carpeta principal donde está el kernel.
```sh
cp -v /boot/config-$(uname -r) .config
```

```sh
make localmodconfig
```

Volver a compilar las llaves privadas de nuestro sistema actual terminará en error. Para evitarlo, se debe ejecutar los siguientes comando:

```sh
# 1
scripts/config --disable SYSTEM_TRUSTED_KEYS
# 2
scripts/config --disable SYSTEM_REVOCATION_KEYS
# 3 Reiniciar las llaves previamente instaladas
scripts/config --set-str CONFIG_SYSTEM_TRUSTED_KEYS ""
# 4 Reiniciar las llaves previamente instaladas
scripts/config --set-str CONFIG_SYSTEM_REVOCATION_KEYS ""
```

Posteriormente, se debe utilizar el siguiente comando para inciar el proceso de compilación:
```sh
fakeroot make -j2
```

## Error 3: CORS en API

Se deben agregar los encabezados de CORS en la response que envía la API, de lo contrario, el frontend no podrá realizar la request exitosamente.

![error-3](./docs/imgs/error-3.png)

### Solución

Se agregó una función para agregar los encabezados de CORS a cada response de cada endpoint:

```c
#include <microhttpd.h>
#include <stdio.h>
void add_cors_headers(struct MHD_Response *response) {
    MHD_add_response_header(response, "Access-Control-Allow-Origin", "*");
    MHD_add_response_header(response, "Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    MHD_add_response_header(response, "Access-Control-Allow-Headers", "Content-Type");
}
```

Ejemplo de uso:

```c
// Función que maneja el endpoint "/memory"
int handle_memory_request(struct MHD_Connection *connection) {
    
    // {Código...}

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
```

## Error 4: Error de variables
Se obtuvieron algunos errores por la declaración de variables. En estos casos, se debe tener especial cuidado debido a la gestión de memoria estricta que se realiza al hacer syscalls. Además de cuidar el tipo de cada dato, evitar declaraciones duplicadas, y el uso de punteros.

![error-4](./docs/imgs/error-4.png)

### Solución
Se solucióno al corregir el nombre de algunas variables en la syscall que obtiene los procesos que más memoria consumen, esto debido a que se habían duplicado. También se modificó el nombre de la variable struct que se copia al espacio de usuario.

```c
SYSCALL_DEFINE1(top_memory_processes, struct process_info __user *, top_procs){
    // { Código... }

    // Copiar el resultado al espacio de usuario
    if (copy_to_user(top_procs, top5, sizeof(top5))) {
        return -EFAULT;
    }
    return 0;
}
```
