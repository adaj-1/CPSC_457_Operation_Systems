#include <sys/mman.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
pthread_mutex_t lock;


int main(){
    if(pthread_mutex_init(&lock, NULL)!=0){
        printf("Mutex failed\n");
        return 0;
    }

    int * i = (int *)mmap(NULL, sizeof(int), PROT_READ|PROT_WRITE, 
        MAP_SHARED|MAP_ANONYMOUS, -1, 0);

    *i = 0;
    int max_size = 6;
    pid_t id = fork();
    if(id ==0){
        while(true){
            pthread_mutex_lock(&lock);
            if(*i< max_size){
                *i = *i+1;
            }
            pthread_mutex_unlock(&lock);
            sleep(5);
        }
        
    }
    else{
        while(true){
            pthread_mutex_lock(&lock);
            if(*i > 0){
                *i = *i-1;
            }
            pthread_mutex_unlock(&lock);
        }

    }


}