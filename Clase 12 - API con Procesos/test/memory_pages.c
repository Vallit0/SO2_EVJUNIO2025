#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>

struct memory_pages_info {
    unsigned long total;
    unsigned long active_pages;
    unsigned long inactive_pages;

    unsigned long active_pages_nr;
    unsigned long inactive_pages_nr;

    unsigned long active_pages_mem;
    unsigned long inactive_pages_mem;
};

int main()
{
    struct memory_pages_info info;

    // Invocar la syscall
    if (syscall(342, &info) == 0)
    {
        printf("Total: %lu\n", info.total);
        printf("Active: %lu\n", info.active_pages);
        printf("Inactive: %lu\n", info.inactive_pages);
        printf("ActiveNR: %lu\n", info.active_pages_nr);
        printf("InactiveNR: %lu\n", info.inactive_pages_nr);
        printf("ActiveMEM: %lu\n", info.active_pages_mem);
        printf("InactiveMEM: %lu\n", info.inactive_pages_mem);
    }
    else
    {
        perror("sys_memorypages syscall failed");
    }

    return 0;
}