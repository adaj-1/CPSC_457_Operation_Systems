#include <stdio.h>
#include <unistd.h>

int main()
{
    printf("Hello\n");
    // create & run child process - a duplicate of parten
    fork();
    // both parent and child will execute the next line
    printf('world.\n');
}

// g++ -o out Exmaple2_Fork1.cpp
// ./out
// Hello
// world.
// world.
