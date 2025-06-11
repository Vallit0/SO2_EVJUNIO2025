# DOCUMENTACION LINUX USAC

<br>

# Llamadas al Sistema para Encriptación y Desencriptación de Archivos

Para implementar la encriptación y desencriptación de un archivo de texto plano, se han añadido dos nuevas llamadas al sistema en el kernel de Linux. A continuación, se describen los pasos para acceder al código fuente del kernel y crear las definiciones correspondientes.


## syscalls_usac.h

Luego, creamos un archivo llamado `syscalls_usac.h` en la carpeta `include/linux` con el siguiente comando:

```bash
cd linux-6.6.46/include/linux
nano syscalls_usac.h
```

En este archivo, se definirán las llamadas al sistema que permitirán encriptar y desencriptar un archivo de texto plano.
```c
#ifndef _SYSCALLS_USAC_H
#define _SYSCALLS_USAC_H
#include <linux/syscalls.h>
#include <linux/kernel.h>

asmlinkage long my_encrypt(const char __user *input_file, const char __user>
asmlinkage long my_decrypt(const char __user *input_file, const char __user>

#endif
```

## my_syscalls.c

creamos un archivo llamado `my_syscalls.c` en la carpeta `kernel` con el siguiente comando:

```bash
cd linux-6.6.46/kernel
nano my_calls.c
```

```c
#include <linux/syscalls.h> 
#include <linux/fs.h> 
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/syscalls_usac.h>

// Estructura para manejar fragmentos de datos
typedef struct {
    unsigned char *data; // Puntero al buffer de datos
    size_t data_size; // Tamaño del buffer de datos
    unsigned char *key; // Puntero al buffer de la clave
    size_t key_size; // Tamaño del buffer de la clave
    size_t start; // Índice de inicio del fragmento
    size_t end; // Índice de fin del fragmento
} DataFragment;

// Estructura para almacenar parámetros de tarea
struct task_parameters {
    DataFragment fragment;
    struct completion done;
};

// Función para realizar la operación XOR en un fragmento de datos
int xor_task(void *arg) {
    struct task_parameters *params = (struct task_parameters *)arg; // Convertir argumento a task_parameters
    DataFragment *fragment = &params->fragment; // Obtener el fragmento de datos
    size_t i;

    printk(KERN_INFO "Hilo iniciado: inicio=%zu, fin=%zu\n", fragment->start, fragment->end); // Registrar inicio del hilo

    // Ejecutar operación XOR en el fragmento de datos
    for (i = fragment->start; i < fragment->end; i++) {
        fragment->data[i] ^= fragment->key[i % fragment->key_size];
    }

    printk(KERN_INFO "Hilo completado: inicio=%zu, fin=%zu\n", fragment->start, fragment->end); // Registrar finalización del hilo
    complete(&params->done); // Señalar que la tarea ha terminado
    return 0;
}

// Función que procesa el archivo de entrada y ejecuta la operación XOR
int process_file(const char *input_path, const char *output_path, const char *key_path, int num_threads) {
    struct file *input_file, *output_file, *key_file; // Punteros a archivos de entrada, salida y clave
    loff_t input_offset = 0, output_offset = 0, key_offset = 0; // Desplazamientos de archivos
    unsigned char *key_buffer, *data_buffer; // Buffers para la clave y los datos
    size_t input_file_size, key_file_size; // Tamaños de archivos de entrada y clave
    struct task_parameters *task_params; // Array de parámetros de tarea
    struct task_struct **threads; // Array de punteros a hilos
    DataFragment *fragments; // Array de fragmentos de datos
    size_t fragment_size, remainder; // Tamaño de cada fragmento y resto
    int i, ret = 0;

    printk(KERN_INFO "Abriendo archivos: entrada=%s, salida=%s, clave=%s\n", input_path, output_path, key_path); // Registrar rutas de archivos

    // Abrir archivos de entrada, salida y clave
    input_file = filp_open(input_path, O_RDONLY, 0);
    output_file = filp_open(output_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    key_file = filp_open(key_path, O_RDONLY, 0);

    // Comprobar errores al abrir archivos
    if (IS_ERR(input_file)) {
        ret = PTR_ERR(input_file);
        printk(KERN_ERR "Error al abrir el archivo de entrada: %d\n", ret);
        goto out;
    }
    if (IS_ERR(output_file)) {
        ret = PTR_ERR(output_file);
        printk(KERN_ERR "Error al abrir el archivo de salida: %d\n", ret);
        goto close_input_file;
    }
    if (IS_ERR(key_file)) {
        ret = PTR_ERR(key_file);
        printk(KERN_ERR "Error al abrir el archivo de clave: %d\n", ret);
        goto close_output_file;
    }

    // Obtener el tamaño del archivo de clave
    key_file_size = i_size_read(file_inode(key_file));
    if (key_file_size <= 0) {
        ret = -EINVAL;
        printk(KERN_ERR "Tamaño de clave inválido: %zu\n", key_file_size);
        goto close_key_file;
    }

    // Asignar memoria para el buffer de clave
    key_buffer = kmalloc(key_file_size, GFP_KERNEL);
    if (!key_buffer) {
        ret = -ENOMEM;
        printk(KERN_ERR "No se pudo asignar memoria para la clave\n");
        goto close_key_file;
    }

    // Leer el archivo de clave en el buffer de clave
    ret = kernel_read(key_file, key_buffer, key_file_size, &key_offset);
    if (ret < 0) {
        printk(KERN_ERR "Error al leer la clave: %d\n", ret);
        goto free_key_buffer;
    }

    // Obtener el tamaño del archivo de entrada
    input_file_size = i_size_read(file_inode(input_file));
    if (input_file_size <= 0) {
        ret = -EINVAL;
        printk(KERN_ERR "Tamaño del archivo de entrada inválido: %zu\n", input_file_size);
        goto free_key_buffer;
    }

    // Asignar memoria para el buffer de datos
    data_buffer = kmalloc(input_file_size, GFP_KERNEL);
    if (!data_buffer) {
        ret = -ENOMEM;
        printk(KERN_ERR "No se pudo asignar memoria para los datos\n");
        goto free_key_buffer;
    }

    // Leer el archivo de entrada en el buffer de datos
    ret = kernel_read(input_file, data_buffer, input_file_size, &input_offset);
    if (ret < 0) {
        printk(KERN_ERR "Error al leer el archivo de entrada: %d\n", ret);
        goto free_data_buffer;
    }

    // Asignar memoria para hilos, parámetros de tarea y fragmentos
    threads = kmalloc(sizeof(struct task_struct *) * num_threads, GFP_KERNEL);
    task_params = kmalloc(sizeof(struct task_parameters) * num_threads, GFP_KERNEL);
    fragments = kmalloc(sizeof(DataFragment) * num_threads, GFP_KERNEL);

    if (!threads || !task_params || !fragments) {
        ret = -ENOMEM;
        printk(KERN_ERR "No se pudo asignar memoria para hilos, parámetros de tarea o fragmentos.\n");
        goto free_data_buffer;
    }

    // Calcular el tamaño de cada fragmento y el resto
    fragment_size = input_file_size / num_threads;
    remainder = input_file_size % num_threads;

    // Crear hilos para procesar cada fragmento
    for (i = 0; i < num_threads; i++) {
        fragments[i].data = data_buffer; // Asignar buffer de datos
        fragments[i].data_size = input_file_size; // Asignar tamaño de datos
        fragments[i].key = key_buffer; // Asignar buffer de clave
        fragments[i].key_size = key_file_size; // Asignar tamaño de clave
        fragments[i].start = i * fragment_size; // Asignar índice de inicio
        fragments[i].end = (i == num_threads - 1) ? (i + 1) * fragment_size + remainder : (i + 1) * fragment_size; // Asignar índice de fin

        printk(KERN_INFO "Creando hilo %d: inicio=%zu, fin=%zu\n", i, fragments[i].start, fragments[i].end); // Registrar creación de hilo

        task_params[i].fragment = fragments[i]; // Asignar fragmento a parámetros de tarea
        init_completion(&task_params[i].done); // Inicializar estructura de finalización

        // Crear y ejecutar el hilo
        threads[i] = kthread_run(xor_task, &task_params[i], "xor_thread_%d", i);
        if (IS_ERR(threads[i])) {
            ret = PTR_ERR(threads[i]);
            printk(KERN_ERR "Error al crear el hilo %d: %d\n", i, ret);
            goto free_all;
        }
    }

    // Esperar a que todos los hilos terminen
    for (i = 0; i < num_threads; i++) {
        printk(KERN_INFO "Esperando a que termine el hilo %d\n", i); // Registrar espera de hilo
        wait_for_completion(&task_params[i].done); // Esperar a que el hilo complete
    }

}

// Llamada al sistema que expone la funcionalidad
SYSCALL_DEFINE3(xor_encrypt, const char __user *, input_path, const char __user *, output_path, const char __user *, key_path) {
    return process_file(input_path, output_path, key_path, 4); // Llamar a process_file con 4 hilos
}

```

### Makefile
Para compilar el módulo del kernel que contiene las llamadas al sistema, se debe agregar una regla al archivo Makefile en la carpeta kernel:

```bash
cd linux-6.6.46/kernel
nano Makefile
```

Agregamos  al final del archivo Makefile:


```bash
obj-y    = fork.o exec_domain.o panic.o \
            cpu.o exit.o softirq.o resource.o \
            sysctl.o capability.o ptrace.o user.o \
            signal.o sys.o umh.o workqueue.o pid.o task_work.o \
            extable.o params.o \
            kthread.o sys_ni.o nsproxy.o \
            notifier.o ksysfs.o cred.o reboot.o \
            async.o range.o smpboot.o ucount.o regset.o ksyms_common.o \
            my_syscalls.o
```

### Encriptación
```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <errno.h>

#define SYS_MY_ENCRYPT 550

// Función para mostrar el uso correcto del programa
void mostrar_uso(const char *nombre_programa) {
    fprintf(stderr, "Uso: %s -p <ruta_archivo_entrada> -o <ruta_archivo_salida> -j <num_hilos> -k <ruta_archivo_clave>\n", nombre_programa);
}

// Función principal
int main(int argc, char *argv[]) {
    char *archivo_entrada = NULL;
    char *archivo_salida = NULL;
    char *archivo_clave = NULL;
    int num_hilos = 0;
    int opcion;

    // Procesar los argumentos de línea de comandos
    while ((opcion = getopt(argc, argv, "p:o:j:k:")) != -1) {
        switch (opcion) {
            case 'p':
                archivo_entrada = optarg;
                break;
            case 'o':
                archivo_salida = optarg;
                break;
            case 'j':
                num_hilos = atoi(optarg);
                break;
            case 'k':
                archivo_clave = optarg;
                break;
            default:
                mostrar_uso(argv[0]);
                return EXIT_FAILURE;
        }
    }

    // Verificar que todos los argumentos necesarios hayan sido proporcionados
    if (!archivo_entrada || !archivo_salida || !archivo_clave || num_hilos <= 0) {
        mostrar_uso(argv[0]);
        return EXIT_FAILURE;
    }

    // Llamada al sistema para encriptar
    long resultado = syscall(SYS_MY_ENCRYPT, archivo_entrada, archivo_salida, archivo_clave, num_hilos);
    if (resultado < 0) {
        perror("Error en la llamada al sistema");
        return EXIT_FAILURE;
    }

    printf("Encriptación exitosa\n");
    return EXIT_SUCCESS;
}

```
### Desencriptación
```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <errno.h>

#define SYS_MY_DECRYPT 551

// Función para mostrar el uso correcto del programa
void mostrar_uso(const char *nombre_programa) {
    fprintf(stderr, "Uso: %s -p <ruta_archivo_entrada> -o <ruta_archivo_salida> -j <num_hilos> -k <ruta_archivo_clave>\n", nombre_programa);
}

// Función principal
int main(int argc, char *argv[]) {
    char *archivo_entrada = NULL;
    char *archivo_salida = NULL;
    char *archivo_clave = NULL;
    int num_hilos = 0;
    int opcion;

    // Procesar los argumentos de la línea de comandos
    while ((opcion = getopt(argc, argv, "p:o:j:k:")) != -1) {
        switch (opcion) {
            case 'p':
                archivo_entrada = optarg;
                break;
            case 'o':
                archivo_salida = optarg;
                break;
            case 'j':
                num_hilos = atoi(optarg);
                break;
            case 'k':
                archivo_clave = optarg;
                break;
            default:
                mostrar_uso(argv[0]);
                return EXIT_FAILURE;
        }
    }

    // Verificar que todos los argumentos necesarios hayan sido proporcionados
    if (!archivo_entrada || !archivo_salida || !archivo_clave || num_hilos <= 0) {
        mostrar_uso(argv[0]);
        return EXIT_FAILURE;
    }

    // Llamada al sistema para desencriptar
    long resultado = syscall(SYS_MY_DECRYPT, archivo_entrada, archivo_salida, archivo_clave, num_hilos);
    if (resultado < 0) {
        perror("Error en la llamada al sistema");
        return EXIT_FAILURE;
    }

    printf("Desencriptación exitosa\n");
    return EXIT_SUCCESS;
}
```

### Configuración básica del kernel

Para que no demoré tanto la compilación del kernel, se realizarán una serie de pasos la cual acelera este proceso de compilación.

<br>

1. luego de descomprimir el archivo que se descargo de _kernel.org_ debemos de ingresar a esa carpeta.
    ```bash
    cd linux-6.6.46
    ```

2. Necesitamos la configuración de nuestro kernel actual en un archivo que se llamara _.config_.
    ```bash
    cp -v /boot/config-$(uname -r) .config
    ```

3. Dado que es una copia de nuestro kernel actual de la maquina, se incluyen varias librerias y drivers los cuales no es de nuestro interes, por lo tanto se descartará algunas llaves que estan dentro del archivo _.config_ que acabamos de crear.
    ```bash
    make localmodconfig

    scripts/config --disable SYSTEM_TRUSTED_KEYS
    scripts/config --disable SYSTEM_REVOCATION_KEYS
    scripts/config --set-str CONFIG_SYSTEM_TRUSTED_KEYS ""
    scripts/config --set-str CONFIG_SYSTEM_REVOCATION_KEYS ""
    ```

### Compilación del kernel
1. Teniendo ya todo configurado, se procederá a la compilación para esto dependerá mucho de la capacidad de la computadora, tanto procesadores como memoria ram. Esto porque variará el tiempo que tardará en realizar la compilación

    ```bash
    # En este caso la maquina en la cual fue compilado, tenia dos núcleos
    fakeroot make -j2
    ```

### Instalación del kernel

1. Para la instalación se debe de ejecutar los siguientes comandos, instalan todos los módulos y cambios que fueron realizados.

    ```bash
    sudo make modules_install
    sudo make install
    ```

2. Hacer reboot para aplicar los cambios
    ```bash
    sudo reboot
    ```

