#include <iostream>
#include <cstdio>
#include <unistd.h>
// Missing some #includes

int main(void)
{
    int fd[2], nbytes;
    pid_t childpid;
    string s = "Hello world!\n";
    char buffer[BUFFER_SIZE];

    pipe(fd);

    if ((childpid = fork()) == -1)
    {
        perror("fork");
        return 1;
    }

    if (childpid == 0)
    {
        // child process closes up input side of pipe
        close(fd[0]);

        // send "string" through the output side of pipe
        write(fd[1], s.cstr(), s.size() + 1);
        return 0;
    }
    else
    {
        // Parent process closes up output side of pipe
        close(fd[1]);

        //Read in a STRING RFOM THE PIPE
        read(fd[0], buffer, sizeof(buffer));
        printf("Recieved string: %s", buffer);
    }

    return 0;
}