// parser for shell

#include <iostream> //defines cout, cin
#include <fstream>  // performs input/output operations on file
#include <string>
#include <vector>
#include <unistd.h> //defines read(), close()
#include <fcntl.h>  //defines open(), O_RDONLY
#include <numeric>
#include <algorithm>
#include <sys/wait.h>

#define MAX_WORD_SIZE 80
#define MAX_LETTER_SIZE 1000
#define BUFFER_SIZE 1024

using namespace std;

enum cmds
{
    my_pipe = '$',
    is_pipe = '|',
    is_output = '>',
    is_input = '<',
    is_background = '&'
};

// parser function
// Citation: Amir Week 3 Tutorial AssignExample1
int parser(vector<string> *parsed_args)
{
    string input;

    while (true)
    {
        getline(cin, input);

        int space_index;
        // command.find(' ') returns an iterator to the first element in the range that is a ' '
        // npos is a constant value w greatest possible value for an element of type size_t
        // while loop searches for ' ' until the last element is reached
        while ((space_index = input.find(' ')) != string::npos)
        {
            // push_back adds new elem at end of vector after the current last elem
            // substr takes string values between command[0] and command[space_index]
            parsed_args->push_back(input.substr(0, space_index));
            // erase removes command[0] to command[space_index] from command
            input.erase(0, space_index + 1);
        }
        // push_back next command
        parsed_args->push_back(input);

        if (parsed_args->front() == "q")
            return 1;
        else
            return 0;
    }
}

// splits up args and redirections
void multi_cmd(vector<string> *parsed_args, vector<int> *redirection_indices, vector<vector<string>> *input, vector<cmds> *redirections)
{
    cout << ">> in multi_cmd%" << endl;
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

            if (parsed_args->at(redirection_index) == "$")
            {
                redirections->push_back(my_pipe);
            }
            else if (parsed_args->at(redirection_index) == "|")
            {
                redirections->push_back(is_pipe);
            }
            else if (parsed_args->at(redirection_index) == ">")
            {
                redirections->push_back(is_output);
            }
            else if (parsed_args->at(redirection_index) == "<")
            {
                redirections->push_back(is_input);
            }
            else
            {
                redirections->push_back(is_background);
            }
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
        input->push_back(next_input);
        // cout<< "cmds front: " << cmds->at(i).front() << endl;
        // cout << "cmds back: " << cmds->at(i).back() << endl;

        j_start = redirection_index + 1;
    }
}

// formats input for execvp call
void format_input(vector<vector<string>> *input, vector<vector<char *>> *argv)
{
    // int pipe_system_call(vector<int> * redirection_indices, vector<vector<string>> * input, vector<cmds> * redirections)

    cout << ">> in format_input%" << endl;

    for (int i = 0; i < input->size(); i++)
    {
        vector<char *> temp;
        for (int j = 0; j < input->at(i).size(); j++)
        {
            const char *args = input->at(i).at(j).c_str();
            temp.push_back((char *)args);
        }
        temp.push_back(nullptr); // nullptr terminator
        argv->push_back(temp);
    }
}

// checks for any commands
void check_input(vector<string> *parsed_args, vector<int> *redirection_indices)
{
    for (auto it = parsed_args->begin(); it != parsed_args->end(); it++)
    {
        if (*it == "|" || *it == "$" || *it == ">" || *it == "<" || *it == ">>" || *it == "<<")
        {
            redirection_indices->push_back(it - parsed_args->begin());
        }
    }
}

// TODO delete this -> used for testing
bool check_pipe(vector<cmds> *redirections)
{
    for (auto it = redirections->begin(); it != redirections->end(); it++)
    {
        if (*it == is_pipe)
        {
            return true; // pipe
        }
    }
    return false; // no pipe
}

// single command system call
int system_call(vector<string> *parsed_args)
{
    cout << ">> in system_call%" << endl;
    vector<char *> argv;
    int status;
    const char *cmd = parsed_args->front().c_str();

    for (int i = 0; i < parsed_args->size(); i++)
    {
        const char *args = parsed_args->at(i).c_str();
        argv.push_back((char *)args);
    }

    argv.push_back(nullptr); // nullptr terminator

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
        status = execvp(cmd, argv.data());
    }
    else if (pid > 0)
    {
        // This means parent is running

        wait(&status);
        waitpid(pid, &status, 0);
    }
    return status;
}

// pipe system call
int pipe_system_call(vector<int> *redirection_indices, vector<vector<char *>> *argv, vector<cmds> *redirections)
{
    cout << ">> in pipe system_call%" << endl;
    // citation: Amir Tutorial AssignExample2 pipe.cpp
    int status;
    int fd_pair[2];

    const char *cmd1 = argv->front().front();
    const char *cmd2 = argv->back().front(); // TODO HARDCODED FOR TWO COMMANDS

    // Create a pipe
    pipe(fd_pair);
    pid_t cpid = fork();

    if (cpid < 0)
    {
        // This means fork failed
        fprintf(stderr, "Fork failed!\n");
        return 1;
    }
    else if (cpid == 0)
    {
        // Close the read end in the child
        close(fd_pair[0]);
        // Bind the write end with the child's output stream
        dup2(fd_pair[1], 1);
        // Invoke the command
        status = execvp(cmd1, argv->front().data());
    }
    else
    {
        // Close the write end in the parent
        close(fd_pair[1]);
        // Bind the read end to parent's input stream
        dup2(fd_pair[0], 0);
        // Invoke the command
        status = execvp(cmd2, argv->back().data());
    }
    return status;
}

// overwrite system call
int overwrite_system_call(vector<int> *redirection_indices, vector<vector<char *>> *argv, vector<cmds> *redirections)
{
    cout << ">> in overwrite_system_call%" << endl;
    int status;
    int fd_pair[2];
    const char *cmd1 = argv->front().front();
    const char *cmd2 = argv->back().front(); // TODO HARDCODED FOR TWO COMMANDS
    // Create a pipe
    pipe(fd_pair);
    pid_t cpid = fork();

    if (cpid < 0)
    {
        // This means fork failed
        fprintf(stderr, "Fork failed!\n");
        return 1;
    }
    else if (cpid == 0)
    {
        // citation Amir AssignExample2 Tutorial dup.cpp

        // overwrite standard output ">"
        if (redirections->front() == is_output)
        {
            if ((fd_pair[0] = open(cmd2, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)) < 0)
            {
                fprintf(stderr, "Output file could not be opened\n");
                return 1;
            }
            // Bind the write end with the child's output stream
            dup2(fd_pair[0], 1); // 1 -> stdout
            // From now on, all my output is redirected to the file
            status = execvp(cmd1, argv->front().data());
            close(fd_pair[0]);
        }

        // overwrite standard input "<"
        if (redirections->front() == is_input)
        {
            if ((fd_pair[1] = open(cmd2, O_RDONLY | O_CREAT, S_IRUSR | S_IRGRP)) < 0)
            {
                fprintf(stderr, "Input file could not be opened\n");
                return 1;
            }
            // Bind the write end with the child's output stream
            dup2(fd_pair[1], 0); // 0 -> stdin
            // From now on, all my input is redirected to the file
            status = execvp(cmd1, argv->front().data());
            close(fd_pair[1]);
        }
    }
    else
    {
        while (!(wait(&status) == cpid))
            ;
    }
    return 0;
}

// https://www.geeksforgeeks.org/making-linux-shell-c/
int main()
{
    // init_JadaShell();
    while (1)
    {
        int status = 0;
        int num_of_redirections;

        vector<string> parsed_args;
        vector<vector<string>> inputs;
        vector<vector<char *>> argv;

        vector<int> redirection_indices;
        vector<cmds> redirections;

        cout << "JadaShell% ";
        status = parser(&parsed_args);
        // flag returns 1 if quit/ error
        // flag returns 0 if command/ running

        if (status == 0)
        {
            check_input(&parsed_args, &redirection_indices);
            // cout << "num of redirections: " << redirection_indices.size() << endl;

            if (redirection_indices.size() == 0) // no io_redirection called
            {
                status = system_call(&parsed_args);
            }
            else
            {
                multi_cmd(&parsed_args, &redirection_indices, &inputs, &redirections);
                format_input(&inputs, &argv);

                /*
                for (int i = 0; i < redirections.size(); i++)
                {

                }
                */

                if (check_pipe(&redirections)) // TODO not always pipe first
                {
                    status = pipe_system_call(&redirection_indices, &argv, &redirections);
                }
                else // hardcoded not correct if else redirection only used to test overwrite system call
                {
                    status = overwrite_system_call(&redirection_indices, &argv, &redirections);
                }
            }
        }
        else
        {
            cout << "Exiting JadaShell%" << endl;
            break;
        }
    }
    return 0;
}