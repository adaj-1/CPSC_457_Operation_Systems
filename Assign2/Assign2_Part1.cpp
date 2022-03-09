#include <stdlib.h>    //standard stuff
#include <sys/mman.h>  //mmap()
#include <stdio.h>     //io
#include <unistd.h>    //sleep
#include <semaphore.h> //semaphore
#include <time.h>      //time
#include <sys/wait.h>  // wait

#define MAX_BUFFER_SIZE 10 // tray size
#define DISH1 1            // Fettuccine Chicken Alfredo Non-Vegan Buffer or Pistachio Pesto Pasta Vegan Buffer
#define DISH2 2            // Garlic Sirloin Steak Non-Vegan Buffer or Avocado Fruit Salad Vegan Buffer

// Non-vegan process
void nv_proceess(int *nv_tray, int *nv_tray_in, int *nv_tray_out, sem_t *nv_mutex, sem_t *nv_empty, sem_t *nv_full, pid_t nv_chef, pid_t nv_cust)
{
    // Child producer process
    if ((nv_chef = fork()) == 0)
    {
        srand((unsigned)time(NULL) % getpid()); // seeding randomness
        while (1)
        {
            int next_produced = 1 + rand() % 2; // randomly select which entree to add to tray

            sem_wait(nv_empty); // empty is the amount of empty slots in the semiphore, wait until empty is not 0
            sem_wait(nv_mutex); // lock

            // CRITICAL SECTION, (ADD TO BUFFER)
            if (next_produced == 1)
            {
                nv_tray[*nv_tray_in] = DISH1;
                printf("Donatello creates non-vegan dish: Fettuccine Chicken Alfredo\n");
            }
            else if (next_produced == 2)
            {
                nv_tray[*nv_tray_in] = DISH2;
                printf("Donatello creates non-vegan dish: Garlic Sirloin Steak\n");
            }

            *nv_tray_in = (*nv_tray_in + 1) % MAX_BUFFER_SIZE;  // updating in pointer

            sem_post(nv_mutex); // unlock
            sem_post(nv_full);

            sleep(1 + rand() % 5); // 1<= sleep <= 5 seconds    this makes it runs faster than consumer
        }
    }
    // Child consumer process
    else if ((nv_cust = fork()) == 0)
    {
        srand((unsigned)time(NULL) % getpid()); // seeding randomness
        while (1)
        {
            sem_wait(nv_full);
            sem_wait(nv_mutex); // lock

            int next_consumed = nv_tray[*nv_tray_out]; // get next consumed item

            // CRITICAL SECTION (TAKE FROM BUFFER)
            if (next_consumed == 1)
            {
                printf("Non-vegan customer removes non-vegan dish: Fettuccine Chicken Alfredo\n");
            }
            else if (next_consumed == 2)
            {
                printf("Non-vegan customer removes non-vegan dish: Garlic Sirloin Steak\n");
            }

            *nv_tray_out = (*nv_tray_out + 1) % MAX_BUFFER_SIZE;    // updating out pointer

            sem_post(nv_mutex); // unlock
            sem_post(nv_empty);

            sleep(10 + rand() % 6); // 10<= sleep <= 15 seconds
        }
    }
}

// Vegan process
void v_proceess(int *v_tray, int *v_tray_in, int *v_tray_out, sem_t *v_mutex, sem_t *v_empty, sem_t *v_full, pid_t v_chef, pid_t v_cust)
{
    // Child producer process
    if ((v_chef = fork()) == 0)
    {
        srand((unsigned)time(NULL) % getpid()); // seeding randomness
        while (1)
        {         
            int next_produced = 1 + rand() % 2; // randomly select which entree to add to tray

            sem_wait(v_empty); // empty is the amount of empty slots in the semiphore, wait until empty is not 0
            sem_wait(v_mutex); // lock

            // CRITICAL SECTION, (ADD TO BUFFER)
            if (next_produced == 1)
            {
                v_tray[*v_tray_in] = DISH1;
                printf("Portecelli creates vegan dish: Pistachio Pesto Pasta\n");
            }
            else if (next_produced == 2)
            {
                v_tray[*v_tray_in] = DISH2;
                printf("Portecelli creates vegan dish: Avocado Fruit Salad\n");
            }

            *v_tray_in = (*v_tray_in + 1) % MAX_BUFFER_SIZE;    // updating in pointer

            sem_post(v_mutex); // unlock
            sem_post(v_full);

            sleep(1 + rand() % 5); // 1<= sleep <= 5 seconds    this makes it runs faster than consumer
        }
    }
    // Child consumer process
    else if ((v_cust = fork()) == 0)
    {
        srand((unsigned)time(NULL) % getpid()); // seeding randomness
        while (1)
        {
            sem_wait(v_full);
            sem_wait(v_mutex); // lock

            int next_consumed = v_tray[*v_tray_out]; // next consumed item

            // CRITICAL SECTION (TAKE FROM BUFFER)
            if (next_consumed == 1)
            {
                printf("Vegan customer removes vegan dish: Pistachio Pesto Pasta\n");
            }
            else if (next_consumed == 2)
            {
                printf("Vegan customer removes vegan dish: Avocado Fruit Salad\n");
            }

            *v_tray_out = (*v_tray_out + 1) % MAX_BUFFER_SIZE;  //updating out pointer

            sem_post(v_mutex); // unlock
            sem_post(v_empty);

            sleep(10 + rand() % 6); // 10<= sleep <= 15 seconds
        }
    }
}

int main()
{
    // Create shared memory for non-vegan tray
    int *nv_tray = (int *)mmap(NULL, MAX_BUFFER_SIZE * sizeof(int *), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    int *nv_tray_in = (int *)mmap(NULL, sizeof(int *), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);  // initialize to index 0
    int *nv_tray_out = (int *)mmap(NULL, sizeof(int *), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0); // initalize to index 0
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
    int *v_tray_in = (int *)mmap(NULL, sizeof(int *), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);  // initialize to index 0
    int *v_tray_out = (int *)mmap(NULL, sizeof(int *), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0); // initalize to index 0
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

    hybrid_cust = fork();
    if (hybrid_cust == -1)
        perror("Error forking at hybrid_cust\n");
    if (hybrid_cust == 0) // hybrid_cust
    {
        srand((unsigned)time(NULL) % getpid()); // seeding randomness
        while (1)
        {
            sem_wait(nv_full);
            sem_wait(v_full);
            sem_wait(nv_mutex); // lock
            sem_wait(v_mutex);  // lock
            
            int nv_next_consumed = nv_tray[*nv_tray_out]; // next consumed item in non-vegan tray
            int v_next_consumed = v_tray[*v_tray_out];    // next consumed item in vegan tray

            if (nv_next_consumed == 1 && v_next_consumed == 1)
            {
                printf("Hybrid customer removes non-vegan dish: Fettuccine Chicken Alfredo, and vegan dish: Pistachio Pesto Pasta\n");
            }
            else if (nv_next_consumed == 2 && v_next_consumed == 2)
            {
                printf("Hybrid customer removes non-vegan dish: Garlic Sirloin Steak, and vegan dish: Avocado Fruit Salad\n");
            }
            else if (nv_next_consumed == 1 && v_next_consumed == 2)
            {
                printf("Hybrid customer removes non-vegan dish: Fettuccine Chicken Alfredo, and vegan dish: Avocado Fruit Salad\n");
            }
            else if (nv_next_consumed == 2 && v_next_consumed == 1)
            {
                printf("Hybrid customer removes non-vegan dish: Garlic Sirloin Steak, and vegan dish: Pistachio Pesto Pasta\n");
            }

            *nv_tray_out = (*nv_tray_out + 1) % MAX_BUFFER_SIZE;    // update out pointer in non-vegan tray
            *v_tray_out = (*v_tray_out + 1) % MAX_BUFFER_SIZE;      // update out pointer in vegan tray

            sem_post(nv_mutex); // unlock
            sem_post(v_mutex);  // unlock
            sem_post(nv_empty);
            sem_post(v_empty);

            sleep(10 + rand() % 6); // 10<= sleep <= 15 seconds
        }
        }
    else
    {
        v_chef = fork();
        if (v_chef == -1)
            perror("Error forking at v_chef");
        if (v_chef == 0) // v_chef process
        {
            v_proceess(v_tray, v_tray_in, v_tray_out, v_mutex, v_empty, v_full, v_chef, v_cust);
        }
        else // parent2
        {
            nv_chef = fork();
            if (nv_chef == -1)
                perror("Error forking at nv_chef");
            if (nv_chef == 0) // nv_chef process
            {
                nv_proceess(nv_tray, nv_tray_in, nv_tray_out, nv_mutex, nv_empty, nv_full, nv_chef, nv_cust);
            }
            else
            {
                while (1)
                {
                    int nv_takenSlots;
                    int v_takenSlots;

                    sem_getvalue(nv_full, &nv_takenSlots);  // checking how full non-vegan tray is
                    sem_getvalue(v_full, &v_takenSlots);    // checking how full vegan tray is

                    printf("\n");
                    printf("Items in the non-vegan tray %d/%d, Items in the vegan tray %d/%d", nv_takenSlots, MAX_BUFFER_SIZE, v_takenSlots, MAX_BUFFER_SIZE);
                    printf("\n");
                    sleep(10); // sleep before updating tray status
                }
                int status;
                waitpid(nv_chef, &status, 0); // wait for nv_chef
            }
            int status;
            waitpid(v_chef, &status, 0); // wait for v_chef
        }
        int status;
        waitpid(hybrid_cust, &status, 0); // wait for hybrid_cust
    }
    exit(0);
}
