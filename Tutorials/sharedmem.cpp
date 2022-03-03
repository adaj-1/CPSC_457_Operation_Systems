#include <sys/mman.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>
#include <pthread.h>
pthread_mutex_t lock;

int main(){
    pthread_mutex_init(&lock, NULL);
    int fd[2];
    pipe(fd);
    pid_t id;
    int * i = (int *) mmap(NULL, sizeof(int), PROT_READ|PROT_WRITE, 
        MAP_SHARED|MAP_ANONYMOUS, -1, 0);

    *i = 0;
    id = fork();

    
    if(id ==0){
        pthread_mutex_lock(&lock);
        int saved = 0;
        pthread_mutex_unlock(&lock);
        while(true){
            pthread_mutex_lock(&lock);
            if(*i == 20){
                break;
            }
            if(*i!=0){
                pthread_mutex_unlock(&lock);
                continue;
            }
            *i = ++saved;
            pthread_mutex_unlock(&lock);
        }
        // FILE * writeend = fdopen(fd[0], "w");
        // fprintf(stdout, "Hello I'm the child\n");
    }
    else{
        // waitpid(id, NULL, 0);
        while(true){
            pthread_mutex_lock(&lock);
            if(*i==20){
                break;
            }
            if(*i==0){
                pthread_mutex_unlock(&lock);
                continue;
            }
            printf("%d\n", *i);
            *i = 0;
            pthread_mutex_unlock(&lock);
        }
        
        // printf("Hello I'm the parent\n");
    }

}