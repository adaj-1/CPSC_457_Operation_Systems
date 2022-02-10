// Example 9: use execvp to execute an user-specified Unix command

#include <unistd.h>
#include <string.h>
#include <cstdio>
#include <iostream>
#include <sys/wait.h>

#define BUFFER_SIZE 1024
#define WRITE_END 1
#define READ_END 0

using namespace std;

int main() 
{
    //Get input string as a char*
    char input[BUFFER_SIZE];
    fgets(input, BUFFER_SIZE, stdin);
    input[strlen(input) -1] = '\0';

    //Split input string into tokens (words), spliting by space
    char* splitter = strtok(input, " ");
    char* words[56];

    int i = 0;
    bool hasDollarSign = false;
    //Loop through all the tokens (words) and save them to a char* array
    while(splitter != NULL)
	{
        words[i] = splitter;
        if (strcmp(words[i], "$") == 0)
            hasDollarSign = true;
        splitter = strtok(NULL, " ");
        i++;
	}
    
    //Make sure the last word is NULL, this is necessary for execvp to accept the input
    words[i] = NULL;

    /*int acc = 0;
    while (words[acc+1] != NULL)
    {
        printf("%s\n", words[acc++]);
    }*/

    if (!hasDollarSign)
    {
        // Run execvp
        execvp(words[0], words);
        //handle all non dollar sign commands
    }
    else
    {
        //Assume that the command is "pwd ls $ cat"
        //In this example, words[] = {"pwd", "ls", "$", "cat", NULL};
        
        //Your job here is to separate words[] into LHS and RHS commands

        char* left[] = {"pwd", "ls"};
        char* right[] = {"cat"};

        int leftNum = sizeof(left)/sizeof(left[0]);
        int rightNum = sizeof(right)/sizeof(right[0]);
        // cmd1 $ cmd2 cmd3
        if (leftNum == 1 && rightNum == 2)
        {

        }
        //  cmd1 cmd2 $ cmd3
        if (leftNum == 2 && rightNum == 1)
        {
            int fds[2];
            pipe(fds);

            pid_t pid1 = fork()

            if (pid1 == -1)
                perror("Error forking");
            if (pid1 == 0 )  //child1 executes cat
            {
                dup2(fds[READ_END], fileno(stdin));
                // anything below here will write to fileno(stdout)
                close(fds[READ_END]);
                close(fds[WRITE_END])
                    // 2
                    // You need to find a way to automate the creation of this char*[]
                    char *left1 = {"pwd", NULL}; // hardcoded
                execvp(left1[0], left1);
                // 3
                perror("Error executing command in child");
                exit(EXIT_FAILURE);
            }
            else    //parent1
            {
                pid_t pid2 = fork();

                if (pid2 == -1)
                    perror("Error forking at child2");
                if (pid2 == 0) // child2 executes ls
                {
                    //5
                    close(fds[READ_END]);
                    close(fds[WRITE_END])
                        // 2

                        // You need to find a way to automate the creation of this char*[]
                    char *left2 = {"ls", NULL}; // hardcoded
                    execvp(left2[0], left2);
                    // 3
                    perror("Error executing command in child");
                    exit(EXIT_FAILURE);
                }
                else    //parent2
                {
                    pid_t pid3 = fork();
                    if(pid3 == -1)
                        perror("Error forking at child3");
                    if(pid3 == 0)   // child3 executes pwd
                    {
                        // Look at this code, how do we change it for child#1 so that it redirects stdin
                        dup2(fds[WRITE_END], fileno(stdout));
                        //anything below here will write to fileno(stdout)
                        close(fds[READ_END]);
                        close(fds[WRITE_END])
                        //2
                        
                        //You need to find a way to automate the creation of this char*[]
                        char*left1 = {"pwd", NULL}; //hardcoded
                        execvp(left1[0], left1);
                        //3
                        perror("Error executing command in child");
                        exit(EXIT_FAILURE);
                    }
                    else    //parent3
                    {
                        //This is the very first thing that runs
                        int status;
                        close(fds[READ_END]);
                        close(fds[WRITE_END])
                        waitpid(pid3, &status, 0);      // wait for child3
                        //1
                    }

                    int status;
                    waitpid(pid2,&status,0);
                    //4
                }
                int status;
                waitpid(pid3, &status, 0);
            }
        }
        // cmd1 cmd2 $ cmd3 cmd4
        if (leftNum == 2 && rightNum == 2)
        {

        }
        
    }
        

    cout << "Hello\n";

    return 0;
}
		