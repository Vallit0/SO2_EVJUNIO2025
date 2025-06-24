#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>

struct page_faults_info {
    unsigned long minorf_nd;
    unsigned long majorf_nd;
    
    unsigned long minorf_tk;
    unsigned long majorf_tk;

    unsigned long minorf_vms;
    unsigned long majorf_vms;
};

int main() {
    struct page_faults_info info;

    // Invocar la syscall
    if (syscall(341, &info) == 0) {
        printf("MinorPFaults ND: %lu\n", info.minorf_nd);
        printf("MajorPFaults ND: %lu\n", info.majorf_nd);
        printf("MinorPFaults TK: %lu\n", info.minorf_tk);
        printf("MajorPFaults TK: %lu\n", info.majorf_tk);
        printf("MinorPFaults VMS: %lu\n", info.minorf_vms);
        printf("MajorPFaults VMS: %lu\n", info.majorf_vms);
    } else {
        perror("sys_pagefaults syscall failed");
    }

    return 0;
}
