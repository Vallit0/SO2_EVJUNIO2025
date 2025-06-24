#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>

struct mem_info {
    unsigned long total_memory;
    unsigned long free_memory;
    unsigned long used_memory;
    unsigned long cached_memory;
};

int main() {
    struct mem_info info;

    // Invocar la syscall
    if (syscall(339, &info) == 0) {
        printf("Total memory: %lu KB\n", info.total_memory);
        printf("Free memory: %lu KB\n", info.free_memory);
        printf("Used memory: %lu KB\n", info.used_memory);
        printf("Cached memory: %lu KB\n", info.cached_memory);
    } else {
        perror("sys_meminfo syscall failed");
    }

    return 0;
}
