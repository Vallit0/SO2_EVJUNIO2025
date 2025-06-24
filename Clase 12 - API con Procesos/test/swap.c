#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>

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

int main() {
    struct swap_info info;

    // Invocar la syscall
    if (syscall(340, &info) == 0) {
        printf("Total SI: %lu\n", info.total_si);
        printf("Free SI: %lu\n", info.free_si);
        printf("Used SI: %lu\n", info.used_si);

        printf("Total SP: %lu\n", info.total_sp);
        printf("Free SP: %lu\n", info.free_sp);
        printf("Used SP: %lu\n", info.used_sp);
        
        printf("Total MEM: %lu\n", info.total_mem);
        printf("Free MEM: %lu\n", info.free_mem);
        printf("Used MEM: %lu\n", info.used_mem);
    } else {
        perror("sys_swapinfo syscall failed");
    }

    return 0;
}