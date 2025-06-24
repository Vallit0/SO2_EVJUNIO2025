#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>

// Definición de estructura para almacenar los procesos
struct process_info {
    pid_t pid;
    unsigned long memory;
};

int main() {
    struct process_info top_procs[5];
    
    // Llamar a la syscall
    if (syscall(342, top_procs) == -1) {
        perror("syscall failed");
        return 1;
    }

    // Imprimir los procesos con más uso de memoria
    for (int i = 0; i < 5; i++) {
        printf("PID: %d, Memory: %lu bytes\n", top_procs[i].pid, top_procs[i].memory);
    }

    return 0;
}