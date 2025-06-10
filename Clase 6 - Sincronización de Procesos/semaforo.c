#include<stdio.h>
#include<string.h>
#include<pthread.h>
#include<stdlib.h>
#include<unistd.h>

typedef struct {
    int value;
    pthread_mutex_t lock;
    pthread_cond_t cond;
} semaforo;

semaforo s;

//init
void semaforo_init(semaforo *s, int value){
    pthread_mutex_init(&(s->lock), NULL);
    pthread_cond_init(&(s->cond), NULL);
    s->value = value;
}

//wait
void semaforo_wait(semaforo *s){
    pthread_mutex_lock(&(s->lock));
    while (s->value <= 0){
        pthread_cond_wait(&(s->cond), &(s->lock));
    }
    s->value--;
    pthread_mutex_unlock(&(s->lock));
}

//signal
void semaforo_signal(semaforo *s){
    pthread_mutex_lock(&(s->lock));
    s->value++;
    pthread_cond_signal(&(s->cond));
    pthread_mutex_unlock(&(s->lock));
}

void* thread_function(void* arg){

    //wait 
    printf("En espera - %s\n", (char*) arg);
    semaforo_wait(&s);
    
    printf("Obtuvo recursos - %s\n", (char*) arg);
    sleep(4);

    //signal 
    printf("Liberando recuros - %s\n", (char*) arg);
    semaforo_signal(&s);
}

int main(){
    
    semaforo_init(&s, 2);

    pthread_t threads[4];

    for(int i = 0; i < 4; i++){
        char *thread_name = (char*) malloc(12*sizeof(char));
        sprintf(thread_name, "Hilo %d", i);
        pthread_create(&threads[i], NULL, thread_function, thread_name);
    }

    for(int i = 0; i < 4; i++){
        pthread_join(threads[i], NULL);
    }
}