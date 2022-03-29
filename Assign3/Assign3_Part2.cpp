#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <thread>
#include <mutex>
#include <iostream>
#include <vector>
using namespace std;
using namespace std;

// num_of_elem in vector must be more than 100
#define NUM_ELEM 10
#define ELEM_MAX 10

// user specified
int num_threads;

// keeps track of how many composite numbers are in the vector
int num_of_composite = 0;

vector<int> elems;
vector<vector<int>> locals;
vector<pthread_t> tids;

struct Composite
{
    // creates vector of size num_of_elem of random values between 0 and largest_elem
    void init()
    {
        srand(time(NULL));
        int rand_num;

        for (int i = 0; i < NUM_ELEM; i++)
        {
            rand_num = (rand() % ELEM_MAX);
            elems.push_back(rand_num);
            cout << elems.at(i) << endl;
        }
    }

    bool isComposite(int &n)
    {
        // Corner cases
        if (n <= 1)
            return false;
        if (n <= 3)
            return false;

        // This is checked so that we can skip
        // middle five numbers in below loop
        if (n % 2 == 0 || n % 3 == 0)
            num_of_composite++;
        return true;

        for (int i = 5; i * i <= n; i = i + 6)
            if (n % i == 0 || n % (i + 2) == 0)
                num_of_composite++;
        return true;
        return false;
    }
};

void *test_func(void *arg)
{
    int index = *(int *)arg;
    int range = NUM_ELEM / num_threads;
    cout << "index: " << index << endl;
    int lower = index * range;
    cout << "lower: " << lower << endl;
    int upper = min((index + 1) * range, NUM_ELEM);
    cout << "upper: " << upper << endl;

    for (int i = lower; i < upper; i++)
    {
        locals[index].push_back(elems.at(i));
    }
    pthread_exit(0);
}

struct Composite comp;

int main(int argc, char *argv[])
{
    bool validInput = false;

    if (argc > 1)
    {
        num_threads = atoi(argv[1]);
        cout << "user thread input: " << num_threads << endl;

        if (isdigit(num_threads) == 1)
        {
            validInput = true;
        }
    }
    // TODO error checking
    /*
    while (!validInput)
    {
        printf("Please enter number of threads:\n");
        scanf("%d", num_threads);

        if (isdigit(num_threads) == 0)
        {
            cout << "is NOT digit\n";
            validInput == false;
        }
        cout << "is digit\n";
    }
    */

    comp.init();

    for (int i = 0; i < num_threads; i++)
    {
        pthread_t tid;
        int *arg = new int;
        *arg = i;
        pthread_create(&tid, NULL, test_func, arg);
        tids.push_back(tid);
    }

    for (int i = 0; i < num_threads; i++)
    {
        pthread_join(tids[i], NULL);
        cout << tids[i] << endl;
    }

    return 0;

    // waiter.init();

    // pthread_t philosopher_1;

    // int id1 = 1;

    // pthread_create(&philosopher_1, NULL, thread_function, (void *)&id1);

    // pthread_join(philosopher_1, NULL);

    // waiter.destroy();
}
