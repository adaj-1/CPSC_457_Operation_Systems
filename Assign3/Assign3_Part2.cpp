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
#define NUM_ELEM 100000000
#define ELEM_MAX 100

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
            rand_num = rand() % ELEM_MAX;
            elems.push_back(rand_num);
        }

        locals.resize(num_threads);
    }

    //---Citation: composite_number.cpp posted on d2l along with assignment---
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
            return true;

        for (int i = 5; i * i <= n; i = i + 6)
            if (n % i == 0 || n % (i + 2) == 0)
                return true;
        return false;
    }
    //---end of citation---
};

struct Composite comp;

void *test_func(void *arg)
{
    //---Citation: Amir Week 7 Histogram-Solution Tutorial---
    int index = *(int *)arg;
    int lower = index * (NUM_ELEM / num_threads);
    int upper = min((index + 1) * (NUM_ELEM / num_threads), NUM_ELEM);
    //---end of citation---

    // evenly splitting elems to be checked for each thread
    for (int i = lower; i < upper; i++)
    {
        locals[index].push_back(elems[i]);
    }

    // begin checking for composites within each thread
    for (auto it = locals[index].begin(); it != locals[index].end(); it++)
    {
        if (comp.isComposite(*it))
        {
            num_of_composite++;
        }
    }

    pthread_exit(0);
}

int main(int argc, char *argv[])
{
    bool validInput = false;

    if (argc > 1)
    {
        num_threads = atoi(argv[1]);

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
    }

    float percent = (num_of_composite / NUM_ELEM);

    cout << "n = " << num_threads << endl;
    cout << "num_of_elems: " << NUM_ELEM << endl;
    cout << "num_of_composite:" << num_of_composite << endl;
    cout << percent << " composite" << endl;

    return 0;
}
