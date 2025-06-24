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

// Buffer para leer el archivo /proc/meminfo
#define MEMINFO_BUFFER_SIZE 4096

// Buffer para leer el archivo /proc/vmstat
#define VMSTAT_BUFFER_SIZE 4096

/*
-------------------------- SYSCALL 1 --------------------------
Uso de memoria: Memoria libre, utilizada y en caché. 
---------------------------------------------------------------
*/

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

/*
-------------------------- SYSCALL 2 --------------------------
Páginas de memoria de swap usadas y libres.
Esta llamada al sistema recupera el número de páginas 
de swap libres y utilizadas.
---------------------------------------------------------------
*/

// Definición de estructura para almacenar la información de swap
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

/*
-------------------------- SYSCALL 3 --------------------------
Cantidad de fallos de página.
Esta llamada al sistema devuelve el número total de fallos de 
página, divididos en fallos de página menores y mayores 
desde el inicio del sistema.
---------------------------------------------------------------
*/


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

/*
-------------------------- SYSCALL 4 --------------------------
Páginas de memoria activas e inactivas.
Esta llamada al sistema proporciona el número de 
páginas de memoria activas e inactivas en el sistema. 
---------------------------------------------------------------
*/

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


/*
-------------------------- SYSCALL 5 --------------------------
Procesos que más memoria utilizan.
Esta llamada al sistema recupera información sobre los 5 
procesos que consumen más memoria en el sistema.
---------------------------------------------------------------
*/


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