#include <iostream>
#include <cstdio>
#include <unistd.h>
#include <???>

using namespace std;

int main()
{
    pid_t pid;
    cout << "Hello World" << endl;

    //fork returns:
    //0 for child
    // >0 for parent (returns child id to parent)
    // -1 for failure
    pid = fork();

    if (pid > 0)
    {
        wait(NULL); // child prints before parent
        cout << "I'm the Parent and the PID of my child is " << pid;
        cout << " and my PID =" << getpid() << endl;
    }
    else
        cout << "I'm the Child with PID: " << getpid() << " and my Parent's PID is " << getppid() << endl;

    cout << "Goodbye World" << endl;

    return 0;
}

// g++ -o out Example3_Fork2.cpp
// ./out
// Hello World
// I'm the Child...
// ... Goodbye World
