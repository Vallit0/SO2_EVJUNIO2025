#include<stdio.h>
#include<string.h>
#include<pthread.h>
#include<stdlib.h>
#include<unistd.h>

pthread_mutex_t lock;

void* thread_function(void* arg){

    //wait 
    pthread_mutex_lock(&lock);
    printf("Inicio - %s\n", (char*) arg);

    sleep(4);

    //signal 
    printf("Fin - %s\n", (char*) arg);
    pthread_mutex_unlock(&lock);
}

int main(){
    
    pthread_mutex_init(&lock, NULL);  // Inicializamos nuestro mutex

    pthread_t t1, t2;

    pthread_create(&t1, NULL, thread_function, "Hilo 1");
    pthread_create(&t2, NULL, thread_function, "Hilo 2");

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    pthread_mutex_destroy(&lock);
}