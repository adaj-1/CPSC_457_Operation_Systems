/*
This is a boilerplate for a possible approach for A4.

This approach uses \textbf{monitors} to handle the synchronization of the queue.
It has two key methods: get_customer and add_customer. These methods should be the only interaction between the producer thread
- the thread reading from the input file adding customers to the queue - and the consumer threads - the 4 threads representing the 4 tables of the dining center.

I will not show any code in these threads, as well, I will not show any code in main()

The main use of this monitor structure, if you so wish to use it, is that it enables the safe addition and deletion of customers in a queue.
 This is definitely required because we have 5 threads possibly wanting to access the queue at the same time; the producer and the 4 consumers (tables).
 The queue is implemented using C++'s std::deque data structure. It is almost like an std::vector except it has functionality for placing/deleting elements
 at both the start and end of the queue. Note that even though it dynamically allocates data, you will not receive bonus points for this unless you implement
 the same functionality manually, that is, without using std::deque.

Hopefully the comments  are self-explanatory. If you have any questions, or find any errors, with the attached .cpp file, please let me know as it will help other students!
*/

#include <iostream>
#include <pthread.h>
#include <string>
#include <unistd.h>
#include <semaphore.h>
#include <deque>
#include <vector>
#include <fstream>
#include <iostream>
#include <mutex>
#include <sys/types.h>
#include <chrono>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <cmath>
using namespace std;

// number of tables
#define N 1

// vector to hold tids
vector<pthread_t> tids;

// input file name
string fname;

// data in file
vector<int> fdata;

// count students
int count_students = 1;

pthread_mutex_t mutex1;

string next_token(string &line)
{
    // Finds the substr until the next ' ' and removes it from line
    // returns line if there is no ' '. returns "" if line is empty
    size_t comma_loc = line.find(" ");
    string result = "";
    if (comma_loc != string::npos)
    {
        result = line.substr(0, comma_loc);
    }
    else
    {
        result = line;
        comma_loc = line.length() - 1;
    }
    line.erase(0, comma_loc + 1);
    return result;
}

struct Customer
{
    int id;
    int arr_time; // TODO added arrival time
    int eating_time_left;
    double wait_start;
};

struct QueueMonitor
{
    // Students in the queue
    std::deque<Customer *> customers;

    // Mutex-semaphore used to restrict threads entering a method in this monitor
    // Keep in mind many threads may be inside a method in this monitor, but at most ONE should be executing (the rest should be waiting)
    sem_t mutex_sem;
    // Mutex-semaphore and counter used to keep track of how many threads are waiting inside a method in this monitor
    sem_t next_sem;
    int next_count;

    // Semaphore and counter to keep track of threads waiting for the 'at least one customer available' condition
    // These should be the table threads waiting on a customer
    sem_t condition_nonempty_sem;
    int condition_nonempty_count;

    void init()
    {
        customers = std::deque<Customer *>();

        // Mutex to gain access to (any) method in this monitor is initialized to 1
        sem_init(&mutex_sem, 0, 1);
        //'Next' semaphore - the semaphore for threads waiting inside a method of this monitor - is initialized to 0 (meaning no threads are initially in a method, as expected)
        sem_init(&next_sem, 0, 0);
        next_count = 0;

        // Condition semaphores are intialized to 0 (meaning no threads are initially waiting on this condition, as is expected)
        sem_init(&condition_nonempty_sem, 0, 0);
        condition_nonempty_count = 0;
    }

    void destroy()
    {
        sem_destroy(&mutex_sem);
        sem_destroy(&condition_nonempty_sem);
    }

    // This is the manual implementation of pthread_cond_wait() using semaphores
    void condition_wait(sem_t &condition_sem, int &condition_count)
    {
        condition_count++;
        if (next_count > 0)
            sem_post(&next_sem);
        else
            sem_post(&mutex_sem);
        sem_wait(&condition_sem);
        condition_count--;
    }

    // This is the manual implementation of pthread_cond_signal() using semaphores
    void condition_post(sem_t &condition_sem, int &condition_count)
    {
        if (condition_count > 0)
        {
            next_count++;
            sem_post(&condition_sem);
            sem_wait(&next_sem);
            next_count--;
        }
    }

    Customer *get_customer()
    {
        // A thread needs mutex access to enter any of this monitors' method!!!
        sem_wait(&mutex_sem);

        // Okay so we got mutex access...but what if the queue is empty?
        while (customers.size() < 1)
            //...Then wait for the 'at least one chopsticks available' semaphore per the waiter-implementation specifications!
            condition_wait(condition_nonempty_sem, condition_nonempty_count);

        // If we're here, then at least one customer is in the queue. Get it.
        Customer *c = customers.front();
        // Now remove it from the queue (this only removes it from the queue - the customer data, currently pointed at by c, will still be there)
        customers.pop_front();

        // TODO deallocate memory

        // If at this point there is at least one customer...
        if (customers.size() >= 1)
        {
            //...post to the 'at least one customer' condition
            condition_post(condition_nonempty_sem, condition_nonempty_count);
        }

        // Threads waiting for next_sem are waiting INSIDE one of this monitor's methods...they get priority!
        if (next_count > 0)
            sem_post(&next_sem);
        // If no such threads exist... simply open up the general-access mutex!
        else
            sem_post(&mutex_sem);

        return c;
    }

    void add_customer(Customer *c)
    {
        // A thread needs mutex access to enter any of this monitors' method!!!
        sem_wait(&mutex_sem);

        // Add the customerm to the queue
        customers.push_back(c);

        // Post to the nonempty queue condition
        condition_post(condition_nonempty_sem, condition_nonempty_count);

        // Threads waiting for next_sem are waiting INSIDE one of this monitor's methods...they get priority!
        if (next_count > 0)
            sem_post(&next_sem);
        // If no such threads exist... simply open up the general-access mutex!
        else
            sem_post(&mutex_sem);
    }
};

struct QueueMonitor queue;

/*Function for the producer thread. There should be 1 of these. It is in charge of adding customers to the end of the queue as they arrive.

  Note, that at least for the round-robin implementation, it does not make sense to add all the customers from the file to the queue right away.
  This is because some customers may run out of their time-quantum and be sent to the back of the queue before other customers even arrive!

  Therefore, this producer should only add customers to the queue AS SOON AS THEY GET HERE. How do we do this? There's many ways, but its all on your hands!
*/
void *producer_function(void *arg)
{
    for (auto it = fdata.begin(); it != fdata.end(); it++)
    {
        struct Customer *student;

        student = new struct Customer;
        student->id = count_students;
        count_students++; // added student counter
        student->arr_time = fdata.at(it - fdata.begin());
        it++; // increment iterator to get eating time
        student->eating_time_left = fdata.at(it - fdata.begin());
        // wait for arrival time before adding student
        sleep(student->arr_time);
        printf("Arrive %d \n", student->id);
        queue.add_customer(student);

        // auto start = chrono::system_clock::now();
        // student.wait_start = start;
    }
    pthread_exit(0);
}

/*Function for the consumer/table thread. There should be 4 of these. It is in charge of taking customers from the queue when the table is free

    Note that for the round-robin implementation, a student's eating_time_left may be larger than their time quantum.
    If this is the case, this thread should update eating_time_left, and add the student back to the end of the queue!

*/
void *consumer_function(void *arg)
{

    // pthread_mutex_lock(&mutex1);
    Customer *cust = queue.get_customer();
    // pthread_mutex_unlock(&mutex1);
    auto start = std::chrono::system_clock::now();
    printf("Sit %d\n", cust->id);

    // round robin preempt

    sleep(cust->eating_time_left);
    auto stop = std::chrono::system_clock::now();
    double turnaround = std::chrono::duration_cast<std::chrono::seconds>(stop - start).count();

    cout.precision(2); // for accuracy to 2 decimal places
    cout << "Leave " << cust->id << " Turnaround: " << turnaround << "s, Wait: " << 0 << "s" << endl;
    // printf("Leave %d, Turnaround: %lf s, Wait: %lf s\n", cust->id, ceil(turnaround), NULL);

    pthread_exit(0);
}

int main(int argc, char *argv[])
{
    // read file name
    if (argc > 1)
    {
        fname = argv[1];
    }

    ifstream file;

    file.open(fname);
    string line;
    if (file.is_open())
    {
        while (true)
        {
            // Now read the data from the file and put it in the vector.
            getline(file, line);
            if (file.eof())
                break;
            string token = next_token(line);
            while (token != "")
            {
                fdata.push_back(stoi(token));
                token = next_token(line);
            }
        }
        file.close();

        // remove round robin time quantum
        fdata.erase(fdata.begin());

        // Print the contents of the data structure
        /*
        for (int l = 0; l < fdata.size(); l++)
        {
            cout << fdata.at(l) << ' ';
            cout << '\n';
        }
        */
    }

    queue.init();

    pthread_mutex_init(&mutex1, NULL);

    pthread_t producer;
    int id_prod = N + 1;
    // create producer thread
    pthread_create(&producer, NULL, producer_function, (void *)&id_prod);
    tids.push_back(producer);
    // join producer thread

    // creating n threads for scheduling students and serving them
    //---Citation: Amir Week 7 Histogram-Solution Tutorial---

    for (int i = 0; i < N; i++)
    {
        pthread_t tid;
        int *arg = new int;
        *arg = i;
        pthread_create(&tid, NULL, consumer_function, arg);
        tids.push_back(tid);
    }

    // joining n threads
    for (int i = 0; i < N + 1; i++)
    {
        pthread_join(tids[i], NULL);
    }
    //---end of citation---

    pthread_mutex_destroy(&mutex1);
    queue.destroy();

    return 1;
}