#include <stdio.h>
#include <pthread.h>
#include <string>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <semaphore.h>
#include <chrono>
#include <vector>
#include <numeric>

// This is a monitor representing the waiter for our A3 Part 1
// This implementation is directly derived from operating systems concepts 10th edition 6.7.2: Implementing a Monitor Using Semaphores
struct WaiterMonitor
{
    // TODO How many chopsticks are available - this should be user input between 5 and 10 (inclusive)
    int chopsticks_available;

    std::vector<double> philospher_eat_time;
    int philospher_1_eat_count;
    int philospher_2_eat_count;
    int philospher_3_eat_count;
    int philospher_4_eat_count;
    int philospher_5_eat_count;
    double average_eat_time = -999;

    // Mutex-semaphore used to restrict threads entering a method in this monitor
    // Keep in mind many threads may be inside a method in this monitor, but at most ONE should be executing (the rest should be waiting)
    sem_t mutex_sem;

    // Mutex-semaphore and counter used to keep track of how many threads are waiting inside a method in this monitor (besides those waiting for a condition)
    sem_t next_sem;
    int next_count;

    // Semaphore and counter to keep track of threads waiting for the 'at least one chopstick available' condition
    // These should be the philosopher threads waiting on the right chopstick
    sem_t condition_can_get_1_sem;
    int condition_can_get_1_count;

    // Semaphore and counter to keep track of threads waiting for the 'at least two chopsticks available' condition
    // These should be the philosopher threads waiting on the left chopstick
    sem_t condition_can_get_2_sem;
    int condition_can_get_2_count;

    void init()
    {

        // Mutex to gain access to (any) method in this monitor is initialized to 1
        sem_init(&mutex_sem, 0, 1);

        //'Next' semaphore - the semaphore for threads waiting inside a method of this monitor - is initialized to 0 (meaning no threads are initially in a method, as is logical)
        sem_init(&next_sem, 0, 0);
        next_count = 0;

        // Condition semaphores are intialized to 0 (meaning no threads are initially waiting on these conditions, as is logical)
        sem_init(&condition_can_get_1_sem, 0, 0);
        condition_can_get_1_count = 0;
        sem_init(&condition_can_get_2_sem, 0, 0);
        condition_can_get_2_count = 0;
    }

    void destroy()
    {
        sem_destroy(&mutex_sem);
        sem_destroy(&condition_can_get_1_sem);
        sem_destroy(&condition_can_get_2_sem);
    }

    // This is the manual implementation of pthread_cond_wait() using semaphores
    void condition_wait(sem_t &condition_sem, int &condition_count)
    {
        // condition count is the number of threads waiting on the condition, increment it since the thread calling this method is about to wait
        condition_count++;
        // If there is a waiting thread INSIDE a method in this monitor, they get priority, so post to that semaphore
        if (next_count > 0)
            sem_post(&next_sem);
        // Otherwise, post to the general entry semaphore (the mutex, that is)
        else
            sem_post(&mutex_sem);
        // Wait for this condition to be posted to (Note that as soon as someone posts to this condition, they will halt as this thread has priority!)
        sem_wait(&condition_sem);
        // If I reach here, I have finished waiting :)
        condition_count--;
    }

    // This is the manual implementation of pthread_cond_signal() using semaphores
    void condition_post(sem_t &condition_sem, int &condition_count)
    {
        // If there are any threads waiting on the condition I want to post...
        if (condition_count > 0)
        {
            //...Then they have priority (they were waiting before me), I shall wait in the next_sem gang
            next_count++;
            // Post to the condition_sem gang so they can continue
            sem_post(&condition_sem);
            // Wait for someone to post to next_sem
            sem_wait(&next_sem);
            next_count--;
        }
    }

    void request_left_chopstick()
    {
        // A thread needs mutex access to enter any of this monitors' method!!!
        sem_wait(&mutex_sem);

        // Okay so we got mutex access...but what if there are less than 2 chopsticks available when I am requesting the left chopstick?...
        while (chopsticks_available < 2)
            //...Then wait for the 'at least two chopsticks available' semaphore per the waiter-implementation specifications!
            condition_wait(condition_can_get_2_sem, condition_can_get_2_count);

        // If we're here, then at least two chopsticks are available, use up one of them
        chopsticks_available--;

        if (chopsticks_available >= 1)
        {
            // If at least a chopstick remains, post to the 'at least one chopstick available' condition
            condition_post(condition_can_get_1_sem, condition_can_get_1_count);

            if (chopsticks_available >= 2)
            {
                // If at least two chopsticks remain, post to the 'at least two chopsticks available' condition
                condition_post(condition_can_get_2_sem, condition_can_get_2_count);
            }
        }

        // Threads waiting for next_sem are waiting INSIDE one of this monitor's methods...they get priority!
        if (next_count > 0)
            sem_post(&next_sem);
        // If no such threads exist... simply open up the general-access mutex!
        else
            sem_post(&mutex_sem);
    }

    void request_right_chopstick()
    {
        sem_wait(&mutex_sem);

        // Okay so we got mutex access...but what are no chopsticks available when I am requesting the right chopstick?...
        while (chopsticks_available == 0)
            //...Then wait for the 'at least one chopsticks available' semaphore per the waiter-implementation specifications!
            condition_wait(condition_can_get_1_sem, condition_can_get_1_count);

        // If we're here, then at least one chopsticks are available, use up one of them
        chopsticks_available--;

        if (chopsticks_available >= 1)
        {
            // If at least a chopstick remains, post to the 'at least one chopstick available' condition
            condition_post(condition_can_get_1_sem, condition_can_get_1_count);

            if (chopsticks_available >= 2)
            {
                // If at least two chopsticks remain, post to the 'at least two chopsticks available' condition
                condition_post(condition_can_get_2_sem, condition_can_get_2_count);
            }
        }

        if (next_count > 0)
            sem_post(&next_sem);
        else
            sem_post(&mutex_sem);
    }

    void return_chopsticks()
    {
        sem_wait(&mutex_sem);

        // Add two chopsticks back to available
        chopsticks_available = +2;

        // Update condition post
        if (chopsticks_available >= 2)
        {
            // If at least two chopsticks remain, post to the 'at least two chopsticks available' condition
            condition_post(condition_can_get_2_sem, condition_can_get_2_count);
        }

        if (next_count > 0)
            sem_post(&next_sem);
        else
            sem_post(&mutex_sem);
    }

    void track_time(int &id, double &time)
    {
        if (id == 1 && philospher_1_eat_count < 3)
        {
            philospher_1_eat_count++;
        }

        if (id == 2 && philospher_2_eat_count < 3)
        {
            philospher_2_eat_count++;
        }

        if (id == 3 && philospher_3_eat_count < 3)
        {
            philospher_3_eat_count++;
        }

        if (id == 4 && philospher_4_eat_count < 3)
        {
            philospher_4_eat_count++;
        }

        if (id == 5 && philospher_5_eat_count < 3)
        {
            philospher_5_eat_count++;
        }

        philospher_eat_time.push_back(time);

        if (philospher_1_eat_count == 3 && philospher_2_eat_count == 3 && philospher_3_eat_count == 3 && philospher_4_eat_count == 3 && philospher_5_eat_count == 3)
        {
            average_eat_time = std::accumulate(philospher_eat_time.begin(), philospher_eat_time.end(), 0.0) / philospher_eat_time.size();
        }
    }
};

struct WaiterMonitor waiter;

// Function for the threads
void *thread_function(void *arg)
{

    int id = *((int *)arg);
    srand(time(NULL) + id);

    for (int i = 0; i < 3; i++)
    {
        printf("Philosopher %d is thinking\n", id);
        int think_time = 1 + (rand() % 5);
        sleep(think_time);

        printf("Philospher %d is hungry\n", id);

        auto start = std::chrono::system_clock::now();

        waiter.request_left_chopstick();

        printf("Philospher %d has picked up left chopstick\n", id);

        // Get the right chopstick
        waiter.request_right_chopstick();

        printf("Philospher %d has picked up right chopstick\nPhilosopher %d is eating\n", id, id);
        // Eat
        auto stop = std::chrono::system_clock::now();
        // Eat time
        sleep(5);

        printf("Philospher %d is done eating\n", id);

        double wait_eat_time = std::chrono::duration_cast<std::chrono::seconds>(stop - start).count();

        // Return our chopsticks
        waiter.return_chopsticks();

        waiter.track_time(id, wait_eat_time);

        // TODO delete print statement wait eat time
        printf("Philospher %d wait eat time: %lf s\n", id, wait_eat_time);
        if (waiter.philospher_1_eat_count == 3 && waiter.philospher_2_eat_count == 3 && waiter.philospher_3_eat_count == 3 && waiter.philospher_4_eat_count == 3 && waiter.philospher_5_eat_count == 3)
        {
            printf("Average wait time to eat is: %lf s\n", waiter.average_eat_time);
        }
    }

    pthread_exit(NULL);
}

bool checkInput(struct WaiterMonitor waiter)
{
    if (waiter.chopsticks_available >= 5 && waiter.chopsticks_available <= 10)
    {
        return true;
    }

    return false;
}

int main(int argc, char *argv[])
{
    bool validInput = false;

    while (!validInput)
    {
        printf("Please enter number of chopsticks:\n");
        scanf("%d", &waiter.chopsticks_available);

        validInput = checkInput(waiter);
    }

    waiter.init();

    pthread_t philosopher_1;
    pthread_t philosopher_2;
    pthread_t philosopher_3;
    pthread_t philosopher_4;
    pthread_t philosopher_5;

    int id1 = 1;
    int id2 = 2;
    int id3 = 3;
    int id4 = 4;
    int id5 = 5;

    pthread_create(&philosopher_1, NULL, thread_function, (void *)&id1);
    pthread_create(&philosopher_2, NULL, thread_function, (void *)&id2);
    pthread_create(&philosopher_3, NULL, thread_function, (void *)&id3);
    pthread_create(&philosopher_4, NULL, thread_function, (void *)&id4);
    pthread_create(&philosopher_5, NULL, thread_function, (void *)&id5);

    pthread_join(philosopher_1, NULL);
    pthread_join(philosopher_2, NULL);
    pthread_join(philosopher_3, NULL);
    pthread_join(philosopher_4, NULL);
    pthread_join(philosopher_5, NULL);

    waiter.destroy();
}
