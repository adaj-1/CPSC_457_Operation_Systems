#include <stdlib.h>    //standard stuff
#include <sys/mman.h>  //mmap()
#include <stdio.h>     //io
#include <unistd.h>    //sleep
#include <semaphore.h> //semaphore
#include <time.h>      //time
#include <pthread.h>   //pthread_create(), pthread_join()

#define MAX_BUFFER_SIZE 10 // tray size

char* nv_tray;
char* v_tray;

char DISH1 = 49;    // 1    Fettuccine Chicken Alfredo Non-Vegan Buffer or Pistachio Pesto Pasta Vegan Buffer
char DISH2 = 50;    // 2    Garlic Sirloin Steak Non-Vegan Buffer or Avocado Fruit Salad Vegan Buffer

//semaphores for non-vegan producer consumer
// This is going to act as a binary semaphore
sem_t nv_mutex;
// These are going to act as counting semaphores
sem_t nv_full;
sem_t nv_empty;

//semaphores for vegan producer consumer
// This is going to act as a binary semaphore
sem_t v_mutex;
// These are going to act as counting semaphores
sem_t v_full;
sem_t v_empty;

void *items_in_tray(void *arg);
void *nv_process(void *arg);
void *v_process(void *arg);
void *nv_cust(void *arg);
void *v_cust(void *arg);
void *hybrid_cust(void *arg);

// in and out pointers for both non-vegan and vegan trays
int nv_tray_in = 0;
int nv_tray_out = 0;
int v_tray_in = 0;
int v_tray_out = 0;

int main()
{
    // Create memory
    nv_tray = (char *)malloc(MAX_BUFFER_SIZE * sizeof(int));
    v_tray = (char *)malloc(MAX_BUFFER_SIZE * sizeof(int));

    // Initialize buffer to be all 0's
    for (int i = 0; i < MAX_BUFFER_SIZE; i++)
    {
        nv_tray[i] = 48;
        v_tray[i] = 48;
    }

    //initialize semaphores
    sem_init(&nv_mutex, 0, 1);
    sem_init(&nv_empty, 0, MAX_BUFFER_SIZE);
    sem_init(&nv_full, 0, 0);
    sem_init(&v_mutex, 0, 1);
    sem_init(&v_empty, 0, MAX_BUFFER_SIZE);
    sem_init(&v_full, 0, 0);

    pthread_t nv_process_id, v_process_id, nv_cust_id, v_cust_id, hybrid_cust_id, items_in_tray_id; // create thread ids

    //create all threads
    int result = pthread_create(&items_in_tray_id, NULL, items_in_tray, NULL);
    if (result != 0)
    {
        printf("Error creating items_in_tray thread\n");
    }
    result = pthread_create(&nv_process_id, NULL, nv_process, NULL);
    if (result != 0)
    {
        printf("Error creating nv_producer thread\n");
    }
    result = pthread_create(&v_process_id, NULL, v_process, NULL);
    if (result != 0)
    {
        printf("Error creating v_producer thread\n");
    }
    result = pthread_create(&nv_cust_id, NULL, nv_cust, NULL);
    if (result != 0)
    {
        printf("Error creating nv_cust thread\n");
    }
    result = pthread_create(&v_cust_id, NULL, v_cust, NULL);
    if (result != 0)
    {
        printf("Error creating v_cust thread\n");
    }
    result = pthread_create(&hybrid_cust_id, NULL, hybrid_cust, NULL);
    if (result != 0)
    {
        printf("Error creating hybrid_cust thread\n");
    }
    
    //join all threads
    pthread_join(items_in_tray_id, NULL);
    pthread_join(nv_process_id, NULL);
    pthread_join(v_process_id, NULL);
    pthread_join(nv_cust_id, NULL);
    pthread_join(v_cust_id, NULL);
    pthread_join(hybrid_cust_id, NULL);

    //free memory
    free(nv_tray);
    free(v_tray);

    pthread_exit(NULL);
}

void *items_in_tray(void *arg)
{
    while (1)
    {
        sleep(10); // sleep before updating tray status
        int nv_takenSlots;
        int v_takenSlots;

        sem_getvalue(&nv_full, &nv_takenSlots); // checking how full non-vegan tray is
        sem_getvalue(&v_full, &v_takenSlots);   // checking how full non-vegan tray is

        printf("\n");
        printf("Items in the non-vegan tray %d/%d, Items in the vegan tray %d/%d", nv_takenSlots, MAX_BUFFER_SIZE, v_takenSlots, MAX_BUFFER_SIZE);
        printf("\n");
       
    }
    return NULL;    // exit thread
}

void *nv_process(void *arg)
{
    srand((unsigned)time(NULL) % gettid()); // seeding randomness
    while (1)
    {
        int next_produced = 1 + rand() % 2;     // randomly select which entree to add to tray

        sem_wait(&nv_empty); // empty is the amount of empty slots in the semiphore, wait until empty is not 0
        sem_wait(&nv_mutex); // lock

        // CRITICAL SECTION, (ADD TO BUFFER)
        if (next_produced == 1)
        {
            nv_tray[nv_tray_in] = DISH1;
            printf("Donatello creates non-vegan dish: Fettuccine Chicken Alfredo\n");
        }
        else if (next_produced == 2)
        {
            nv_tray[nv_tray_in] = DISH2;
            printf("Donatello creates non-vegan dish: Garlic Sirloin Steak\n");
        }

        nv_tray_in = (nv_tray_in + 1) % MAX_BUFFER_SIZE; // updating in pointer

        sem_post(&nv_mutex); // unlock
        sem_post(&nv_full);

        sleep(1 + rand() % 5); // 1<= sleep <= 5 seconds    this makes it runs faster than consumer
    }
    return NULL; // exit thread
}
void *v_process(void *arg)
{
    srand((unsigned)time(NULL) % gettid()); // seeding randomness
    while (1)
    {
        int next_produced = 1 + rand() % 2; // randomly seelct which entree to add to tray

        sem_wait(&v_empty); // empty is the amount of empty slots in the semiphore, wait until empty is not 0
        sem_wait(&v_mutex); // lock

        // CRITICAL SECTION, (ADD TO BUFFER)
        if (next_produced == 1)
        {
            v_tray[v_tray_in] = DISH1;
            printf("Portecelli creates vegan dish: Pistachio Pesto Pasta\n");
        }
        else if (next_produced == 2)
        {
            v_tray[v_tray_in] = DISH2;
            printf("Portecelli creates vegan dish: Avocado Fruit Salad\n");
        }

        v_tray_in = (v_tray_in + 1) % MAX_BUFFER_SIZE;    // updating in pointer

        sem_post(&v_mutex); // unlock
        sem_post(&v_full);

        sleep(1 + rand() % 5); // 1<= sleep <= 5 seconds    this makes it runs faster than consumer
    }
    return NULL; // exit thread
}
void *nv_cust(void *arg)
{
    srand((unsigned)time(NULL) % gettid()); // seeding randomness
    while (1)
    {
        sem_wait(&nv_full);
        sem_wait(&nv_mutex); // lock

        // next consumed item
        int next_consumed = nv_tray[nv_tray_out];   // get next consumed item

        // CRITICAL SECTION (TAKE FROM BUFFER)
        if (next_consumed == 49)
        {
            printf("Non-vegan customer removes non-vegan dish: Fettuccine Chicken Alfredo\n");
        }
        else if (next_consumed == 50)
        {
            printf("Non-vegan customer removes non-vegan dish: Garlic Sirloin Steak\n");
        }

        nv_tray_out = (nv_tray_out + 1) % MAX_BUFFER_SIZE;  // updating out pointer

        sem_post(&nv_mutex); // unlock
        sem_post(&nv_empty);

        sleep(10 + rand() % 6); // 10<= sleep <= 15 seconds
    }
    return NULL; // exit thread
}
void *v_cust(void *arg)
{
    srand((unsigned)time(NULL) % gettid()); // seeding randomness
    while (1)
    {
        sem_wait(&v_full);
        sem_wait(&v_mutex); // lock

        int next_consumed = v_tray[v_tray_out]; // next consumed item

        // CRITICAL SECTION (TAKE FROM BUFFER)
        if (next_consumed == 49)
        {
            printf("Vegan customer removes vegan dish: Pistachio Pesto Pasta\n");
        }
        else if (next_consumed == 50)
        {
            printf("Vegan customer removes vegan dish: Avocado Fruit Salad\n");
        }

        v_tray_out = (v_tray_out + 1) % MAX_BUFFER_SIZE;    // updating out pointer

        sem_post(&v_mutex); // unlock
        sem_post(&v_empty);

        sleep(10 + rand() % 6); // 10<= sleep <= 15 seconds
    }
    return NULL; // exit thread
}
void *hybrid_cust(void *arg)
{
    srand((unsigned)time(NULL) % gettid()); // seeding randomness
    while (1)
    {
        sem_wait(&nv_full);
        sem_wait(&v_full);
        sem_wait(&nv_mutex); // lock
        sem_wait(&v_mutex);  // lock
        
        int nv_next_consumed = nv_tray[nv_tray_out]; // next consumed item in non-vegan tray
        int v_next_consumed = v_tray[v_tray_out];    // next consumed item in vegan tray

        if (nv_next_consumed == 49 && v_next_consumed == 49)
        {
            printf("Hybrid customer removes non-vegan dish: Fettuccine Chicken Alfredo, and vegan dish: Pistachio Pesto Pasta\n");
        }
        else if (nv_next_consumed == 50 && v_next_consumed == 50)
        {
            printf("Hybrid customer removes non-vegan dish: Garlic Sirloin Steak, and vegan dish: Avocado Fruit Salad\n");
        }
        else if (nv_next_consumed == 49 && v_next_consumed == 50)
        {
            printf("Hybrid customer removes non-vegan dish: Fettuccine Chicken Alfredo, and vegan dish: Avocado Fruit Salad\n");
        }
        else if (nv_next_consumed == 50 && v_next_consumed == 49)
        {
            printf("Hybrid customer removes non-vegan dish: Garlic Sirloin Steak, and vegan dish: Pistachio Pesto Pasta\n");
        }

        nv_tray_out = (nv_tray_out + 1) % MAX_BUFFER_SIZE; // update out pointer in non-vegan tray
        v_tray_out = (v_tray_out + 1) % MAX_BUFFER_SIZE;   // update out pointer in vegan tray

        sem_post(&nv_mutex); // unlock
        sem_post(&v_mutex);  // unlock
        sem_post(&nv_empty);
        sem_post(&v_empty);

        sleep(10 + rand() % 6); // 10<= sleep <= 15 seconds
    }
    return NULL; // exit thread
}
