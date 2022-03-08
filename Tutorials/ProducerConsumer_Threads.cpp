#include <stdlib.h> //standard stuff
#include <sys/mman.h> //mmap()
#include <stdio.h> //io
#include <unistd.h> //sleep
#include <semaphore.h> //semaphore
#include <time.h> //time
#include<pthread.h> //pthread_create(), pthread_join()

#define BUFFER_SIZE 4

char* buffer;

void printBuffer()
{
    printf("[");
    for (int i = 0; i < BUFFER_SIZE; i++)
    {
        printf("%c", buffer[i]);
    }
    printf("]\n");
}

//This is going to act as a binary semaphore
sem_t mutex;
//These are going to act as counting semaphores
sem_t full;
sem_t empty;

void *producer(void* arg);
void *consumer(void* arg);

int in = 0;
int out = 0;

int main()
{
    //Create memory
    buffer = (char*)malloc(BUFFER_SIZE*sizeof(int));

    //Initialize buffer to be all 0's
    for (int i = 0; i < BUFFER_SIZE; i++)
    {
        buffer[i] = 48;
    }

    sem_init(&mutex, 0, 1);
    sem_init(&empty, 0, BUFFER_SIZE);
    sem_init(&full, 0, 0);

    pthread_t producer_id, consumer_id;

    int result = pthread_create(&producer_id, NULL, producer, NULL);
    if (result != 0)
    {
        printf("Error creating producer thread\n");
    }

    result = pthread_create(&consumer_id, NULL, consumer, NULL);
    if (result != 0)
    {
        printf("Error creating consumer thread\n");
    }

    pthread_join(producer_id, NULL);
    pthread_join(consumer_id, NULL);

    free(buffer);

    exit(0);
}

void* producer(void* arg)
{
    while(1)
    {
        sem_wait(&empty);
        sem_wait(&mutex);
        
		//Produce a '1' at index in, increment in (looping around if needed)
        char randLowerCase = 49;
        buffer[in] = randLowerCase;
        in = (in+1)%BUFFER_SIZE;

        printf("Producer produced, buffer: " ); printBuffer();
        
        sem_post(&full);
        sem_post(&mutex);

        //Sleep between 0 and 5 seconds
        srand(time(NULL));
        sleep(rand()% 5);
    }
}

void *consumer(void* arg)
{
    while(1)
    {
        sem_wait(&full);
        sem_wait(&mutex);
        
		//consume a '1' at index out, increment out (looping if needed)
        char lowerCaseLetter = buffer[out];
        buffer[out] = 48;
        out = (out+1)%BUFFER_SIZE;

        printf("Consumer consumed, buffer: " ); printBuffer();

        sem_post(&mutex);
        sem_post(&empty);

        //Sleep between 3 and 8 seconds
        srand(time(NULL));
        sleep(3 + rand()% 5);
    }
}