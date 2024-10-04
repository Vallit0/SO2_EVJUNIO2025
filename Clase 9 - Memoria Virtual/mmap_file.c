#include <stdio.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int main(){

    const char *filepath = "/home/desquivel/Documents/Clase9/my_file";
    int fd = open(filepath, O_RDWR);
    if(fd < 0){
        perror("No se pudo abrir el archivo");
        return 1;
    }

    struct stat statbuf;
    int err = fstat(fd, &statbuf);
    if(err < 0){
        perror("No se pudo abrir el archivo");
        exit(2);
    }

    char *ptr = mmap(NULL, statbuf.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(ptr == MAP_FAILED){
        perror("Fallo mmap");
        return 1;
    }

    close(fd);

    printf("Leer archivo:\n");
    printf("%s\n", (char *) ptr);

    printf("\n");

    printf("Invertir archivo:\n");
    for(size_t i = 0; i < statbuf.st_size/2; i++){
        int j = statbuf.st_size - i - 1;
        int k = ptr[i];

        ptr[i] = ptr[j];
        ptr[j] = k;
    }

    printf("%s\n", (char *) ptr);

    int err2 = munmap(ptr, statbuf.st_size);
    if(err2 != 0){
        perror("Fallo munmap");
        return 1;
    }
}