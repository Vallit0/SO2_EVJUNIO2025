#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>

void main(){

    pid_t pid = fork(); //Se realiza el fork
    
    if(pid == -1){
        perror("fork");
        exit(1);
    }
    
    if(pid){

        printf("Soy el proceso padre\n");

        /*Se espera a que el proceso hijo termine*/
        int status;
        wait(&status);

        /*Se imprime el codigo de salida del proceso hijo*/
        if(WIFEXITED(status)){
            printf("\nEl proceso hijo termino con estado: %d\n", status);
        } else {
            printf("Ocurrio un error al esperar el proceso hijo");
        }

        printf("Terminando proceso padre\n");
    
    }else{
        /*Se escriben los argumentos para el proceso hijo*/
        char *arg_Ptr[4];
        arg_Ptr[0] = " child.c";
        arg_Ptr[1] = " Hola! ";
        arg_Ptr[2] = " Soy el proceso hijo! ";
        arg_Ptr[3] = NULL; // El ultimo indice de argv siempre debe de ser NULL

        /*Se ejecuta el ejecutable del proceso hijo*/
        execv("/home/desquivel/Documents/Clase2/Ejemplos/child.bin", arg_Ptr);
        
    }
}
