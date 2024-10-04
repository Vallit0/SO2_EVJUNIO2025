#include <stdio.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define SIZE 5

int main(){

    int *ptr = mmap(NULL, sizeof(int)*SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
    if(ptr == MAP_FAILED){
        perror("Fallo mmap");
        return 1;
    }

    printf("%p\n", ptr);

    for(int i = 0; i < SIZE; i++){
        ptr[i] = i * 10;
    }

    printf("Elements: ");
    for(int i = 0; i < SIZE; i++){
        printf("[%d] ", ptr[i]);
    }

    printf("\n");

    int err = munmap(ptr, sizeof(int)*SIZE);
    if(err != 0){
        perror("Fallo munmap");
        return 1;
    }

}