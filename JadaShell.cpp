// parser for shell

#include <iostream> //defines cout, cin
#include <fstream>  // performs input/output operations on file
#include <string>
#include <vector>
#include <unistd.h> //defines read(), close()
#include <fcntl.h>  //defines open(), O_RDONLY
#include <numeric>
#include <algorithm>
#include<sys/wait.h>

#define MAX_WORD_SIZE 80
#define MAX_LETTER_SIZE 1000
#define BUFFER_SIZE 1024


using namespace std;

// parser function
// Citation: Amir Week 3 Tutorial AssignExample1
int parser(vector<string> *parsed_args)
{
    string command;

    while (true)
    {
        getline(cin, command);

        int space_index;
        // command.find(' ') returns an iterator to the first element in the range that is a ' '
        // npos is a constant value w greatest possible value for an element of type size_t
        // while loop searches for ' ' until the last element is reached
        while ((space_index = command.find(' ')) != string::npos)
        {
            // push_back adds new elem at end of vector after the current last elem
            // substr takes string values between command[0] and command[space_index]
            parsed_args->push_back(command.substr(0, space_index));
            // erase removes command[0] to command[space_index] from command
            command.erase(0, space_index + 1);
        }
        // push_back next command
        parsed_args->push_back(command);

        if (parsed_args->front() == "q")
            return 1;
        else
            return 0;
    }
}

void multi_cmd(vector<string> *parsed_args, vector<int> *redirection_indices, vector<vector<string>> *cmds, vector<string> *redirections)
{
    cout << "In multi_cmd" << endl;
    int j_start = 0;
    int redirection_index = 0;

    for (int i = 0; i < redirection_indices->size() + 1; i++)
    {
        vector<string> next_input;
        vector<string> next_parsed_args;

        // checks if added final redirection already
        if (j_start != redirection_indices->back() + 1)
        {
            redirection_index = redirection_indices->at(i);
            redirections->push_back(parsed_args->at(redirection_index));
        }
        // if already added final redirection, get final command
        else
        {
            redirection_index = parsed_args->size();
        }

        for (int j = j_start; j < redirection_index; j++)
        {
            next_input.push_back(parsed_args->at(j));
            // cout << "next_input: " << parsed_args->at(j) << endl;
        }
        cmds->push_back(next_input);
        // cout<< "cmds front: " << cmds->at(i).front() << endl;
        // cout << "cmds back: " << cmds->at(i).back() << endl;

        j_start = redirection_index + 1;
    }
}

void check_commands(vector<string> *parsed_args, vector<int> *redirection_indices)
{
    for (auto it = parsed_args->begin(); it != parsed_args->end(); it++)
    {
        if (*it == "$" || *it == ">" || *it == "<" || *it == ">>" || *it == "<<")
        {
            redirection_indices->push_back(it - parsed_args->begin());
        }
    }    
}

bool check_pipe(vector<string> *redirections)
{
    for (auto it = redirections->begin(); it != redirections->end(); it++)
    {
        if (*it == "$")
        {
            return true;   //pipe
        }
    }
    return false;   //no pipe
}

int system_call(vector<string> *parsed_args)
{
    cout << "in system_call" << endl;
    vector<char *> argv;

    const char *cmd = parsed_args->front().c_str();

    for (int i = 0; i < parsed_args->size(); i++)
    {
        const char *args = parsed_args->at(i).c_str();
        argv.push_back((char *)args);
    }

    argv.push_back(nullptr);  // nullptr terminator

    // following code taken from Amir tutorial AssignExample 2 fork-execvp.cpp
    int pid = fork();
    // From now on we have two process running the same code
    if (pid < 0)
    {
        // This means fork failed
        fprintf(stderr, "Fork failed!\n");
        return 1;
    }
    else if (pid == 0)
    {
        // This means child is running
        int ret_stat = execvp(cmd, argv.data());
        printf("I am the child; the command returned %d\n", ret_stat);
    }
    else if (pid > 0)
    {
        // This means parent is running
        int ret_stat;
        wait(&ret_stat);
        waitpid(pid, &ret_stat, 0);
        printf("I am the parent; the child returned %d\n", ret_stat);
    }
    return 0;
}

int pipe_system_call(vector<int> *redirection_indices, vector<vector<string>> *cmds, vector<string> *redirections)
{
    cout << "in pipe system_call" << endl;
    vector<vector<char *>> argv;
    //vector<char *> redirections;

    const char *cmd1 = cmds->front().front().c_str();
    const char *cmd2 = cmds->at(1).front().c_str();

    for (int i = 0; i < cmds->size(); i++)
    {
        vector<char *> input;
        for (int j = 0; j < cmds->at(i).size(); j++)
        {
            const char *args = cmds->at(i).at(j).c_str();
            input.push_back((char *)args);
        }
        input.push_back(nullptr);   //nullptr terminator
        argv.push_back(input);
    }

    // citation: Amir Tutorial AssignExample2 pipe.cpp
    int status;
    int fd_pair[2];
    // Create a pipe
    pipe(fd_pair);
    pid_t cpid = fork();
    if (cpid == 0)
    {
        // Close the read end in the child
        close(fd_pair[0]);
        // Bind the write end with the child's output stream
        dup2(fd_pair[1], 1);
        // Invoke the command
        status = execvp(cmd1, argv.front().data());
    }
    else if (cpid > 0)
    {
        // Close the write end in the parent
        close(fd_pair[1]);
        // Bind the read end to parent's input stream
        dup2(fd_pair[0], 0);
        // Invoke the command
        status = execvp(cmd2, argv.back().data());
    }
    return status;
}

// Citation: Alex's Tutorial Example1_OpenClose.cpp
// use this for working with I/O Redirection
void open_close(vector<string> *parsed_args)
{
    cout << "in open_close" << endl;

    // char *pathname = "/home/ugd/jada.li/.vscode-server/bin/899d46d82c4c95423fb7e10e68eba52050e30ba3/bin:/home/ugd/jada.li/.vscode-server/bin/899d46d82c4c95423fb7e10e68eba52050e30ba3/bin:/usr/share/ucalgary/bin:/usr/lib64/ccache:/usr/local/bin:/usr/bin:/usr/local/sbin:/usr/sbin:/usr/local/cuda/bin/";

    // pathname += cmd[0];
    //  search directory
    //  Const keyword in C++:
    //  This means that the pointer is pointing to a const variable
    //  Convert a string to a char* - use method c_str() to get C string version

    /*
    // open an existing file
    int fd = open(pathname,O_RDONLY);

    char buffer[BUFFER_SIZE];
    if (read(fd,buffer,BUFFER_SIZE) > 0)
    {
        // read stuff, print it screen
        string s(buffer);
        cout << s << endl;
    }
    else
        cout << "Couldn't read anything" << endl;

    close(fd); // Always close the file!!
    */
}


// https://www.geeksforgeeks.org/making-linux-shell-c/
int main()
{
    // init_JadaShell();
    while (1)
    {    
        int flag = 0;
        int num_of_redirections;
        vector<string> parsed_args;
        vector<int> redirection_indices;
        vector<vector<string>> cmds;
        vector<string> redirections;

        cout << "JadaShell> ";
        flag = parser(&parsed_args);
        // flag returns 1 if quit/ error
        // flag returns 0 if command/ running

        if (flag == 0)
        {
            
            check_commands(&parsed_args, &redirection_indices);
            //cout << "num of redirections: " << redirection_indices.size() << endl;

            if (redirection_indices.size() == 0) // no io_redirection called
            {
                flag = system_call(&parsed_args);
            }
            else
            {
                multi_cmd(&parsed_args, &redirection_indices, &cmds, &redirections);
                if (check_pipe(&redirections)) // TODO not always pipe first
                {
                    flag = pipe_system_call(&redirection_indices, &cmds, &redirections);    
                }
            } 
        }
        else
        {
            cout << "Exiting JadaShell" << endl;
            break;
        }
    }
    return 0;
}