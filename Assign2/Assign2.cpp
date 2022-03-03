#include <stdlib.h>    //standard stuff
#include <sys/mman.h>  //mmap()
#include <stdio.h>     //io
#include <unistd.h>    //sleep
#include <semaphore.h> //semaphore
#include <time.h>      //time

#define MAX_BUFFER_SIZE 10 // tray size
#define dish1 1            // Fettuccine Chicken Alfredo Non-Vegan Buffer or Pistachio Pesto Pasta Vegan Buffer
#define dish2 2            // Garlic Sirloin Steak Non-Vegan Buffer or Avocado Fruit Salad Vegan Buffer

int main()
{
    // Create shared memory for non-vegan tray
    int *nv_tray = (int *)mmap(NULL, MAX_BUFFER_SIZE * sizeof(int *), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    int *nv_tray_in = (int *)mmap(NULL, MAX_BUFFER_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);  // initialize to index 0
    int *nv_tray_out = (int *)mmap(NULL, MAX_BUFFER_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0); // initalize to index 0
    *nv_tray_in = 0;
    *nv_tray_out = 0;
    // Create shared semaphore for non-vegan tray
    // This is going to act as a binary sempahore
    sem_t *nv_mutex = (sem_t *)mmap(NULL, MAX_BUFFER_SIZE * sizeof(sem_t *), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    // This is going to act as a counting semaphore
    sem_t *nv_full = (sem_t *)mmap(NULL, MAX_BUFFER_SIZE * sizeof(sem_t *), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    sem_t *nv_empty = (sem_t *)mmap(NULL, MAX_BUFFER_SIZE * sizeof(sem_t *), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    // Initializing semaphores
    sem_init(nv_mutex, 1, 1); // 1 is unlocked 0 is locked, last param
    sem_init(nv_full, 1, 0);
    sem_init(nv_empty, 1, MAX_BUFFER_SIZE);

    // Create shared memory for vegan tray
    int *v_tray = (int *)mmap(NULL, MAX_BUFFER_SIZE * sizeof(int *), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    int *v_tray_in = (int *)mmap(NULL, MAX_BUFFER_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);  // initialize to index 0
    int *v_tray_out = (int *)mmap(NULL, MAX_BUFFER_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0); // initalize to index 0
    *v_tray_in = 0;
    *v_tray_out = 0;
    // Create shared semaphore for vegan tray
    // This is going to act as a binary sempahore
    sem_t *v_mutex = (sem_t *)mmap(NULL, MAX_BUFFER_SIZE * sizeof(sem_t *), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    // This is going to act as a counting semaphore
    sem_t *v_full = (sem_t *)mmap(NULL, MAX_BUFFER_SIZE * sizeof(sem_t *), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    sem_t *v_empty = (sem_t *)mmap(NULL, MAX_BUFFER_SIZE * sizeof(sem_t *), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    // Initializing semaphores
    sem_init(v_mutex, 1, 1); // 1 is unlocked 0 is locked, last param
    sem_init(v_full, 1, 0);
    sem_init(v_empty, 1, MAX_BUFFER_SIZE);

    // Fork
    pid_t nv_chef;     // donatello
    pid_t v_chef;      // portecelli
    pid_t nv_cust;     // C1, non-vegan dishes
    pid_t v_cust;      // C2, vegan dishes
    pid_t hybrid_cust; // C3, one of each dishes

    // Child producer process
    if ((nv_chef = fork()) == 0)
    {
        while (1)
        {
            // next produced item
            srand(time(0));
            int next_produced = 1 + rand() % 2;
            printf("next_produced: %d ", next_produced);

            sem_wait(nv_empty); // empty is the amount of empty slots in the semiphore, wait until empty is not 0
            sem_wait(nv_mutex); // lock

            // CRITICAL SECTION, (ADD TO BUFFER)
            if (next_produced == 1)
            {
                nv_tray[*nv_tray_in] = dish1;
                printf("Donatello creates non-vegan dish: Fettuccine Chicken Alfredo\n");
            }
            else if (next_produced == 2)
            {
                nv_tray[*nv_tray_in] = dish2;
                printf("Donatello creates non-vegan dish: Garlic Sirloin Steak\n");
            }

            *nv_tray_in = (*nv_tray_in + 1) % MAX_BUFFER_SIZE;

            sem_post(nv_mutex); // unlock
            sem_post(nv_full);

            sleep(1 + rand() % 5); // 1<= sleep <= 5 seconds    this makes it runs faster than consumer
        }
    }
    // Child consumer process
    else if ((nv_cust = fork()) == 0)
    {
        while (1)
        {
            sem_wait(nv_full);
            sem_wait(nv_mutex); // lock

            srand(time(0));
            // next consumed item
            int next_consumed = nv_tray[*nv_tray_out];
            printf("next_consumed: %d ", next_consumed);

            // CRITICAL SECTION (TAKE FROM BUFFER)
            if (next_consumed == 1)
            {
                printf("Non-vegan customer removes non-vegan dish: Fettuccine Chicken Alfredo\n");
            }
            else if (next_consumed == 2)
            {
                printf("Non-vegan customer removes non-vegan dish: Garlic Sirloin Steak\n");
            }

            *nv_tray_out = (*nv_tray_out + 1) % MAX_BUFFER_SIZE;

            sem_post(nv_mutex); // unlock
            sem_post(nv_empty);

            sleep(10 + rand() % 6); // 10<= sleep <= 15 seconds
        }
    }
    // Main process
    else
    {
        while (1)
        {
            int takenSlots;
            sem_getvalue(nv_full, &takenSlots);
            printf("Items in the non-vegan tray %d/%d\n", takenSlots, MAX_BUFFER_SIZE);
            sleep(10); // TODO check if this is the correct sleep time
        }
    }

    exit(0);
}
