// parser for shell

#include <iostream> //defines cout, cin
#include <fstream>  //performs input/output operations on file
#include <string>
#include <vector>
#include <unistd.h> //defines read(), close()
#include <fcntl.h>  //defines open(), O_RDONLY
#include <numeric>
#include <algorithm>
#include <sys/wait.h>

#define WRITE_END 1
#define READ_END 0

using namespace std;

// used to indicate pipes or input/output redirections
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

        if (parsed_args->front() == "q") // shell exit command
            return 1;
        else
            return 0;
    }
}

// splits up args and redirections
void multi_cmd(vector<string> *parsed_args, vector<int> *redirection_indices, vector<vector<string>> *input, vector<cmds> *redirections)
{
    cout << ">> in multi_cmd%" << endl;
    int j_start = 0;           // to keep track of index for commands and arguments
    int redirection_index = 0; // index of input/output redirections or pipes or background

    for (int i = 0; i < redirection_indices->size() + 1; i++)
    {
        vector<string> next_input;
        vector<string> next_parsed_args;

        // adds input/output redirections, pipes, and backgrounds to vector<cmds> redirections
        if (j_start != redirection_indices->back() + 1) // checks if added final redirection already
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
        else // if already added final redirection, get final command
        {
            redirection_index = parsed_args->size();
        }

        // adds commands and their arguments into vector<vector<string>> input
        for (int j = j_start; j < redirection_index; j++)
        {
            next_input.push_back(parsed_args->at(j));
        }
        input->push_back(next_input);
        j_start = redirection_index + 1; // update next command index (skip over input/output redirections, pipes, and background symbols)
    }
}

// formats vector<vector<string>>input for execvp call and error checking
void format_input(vector<vector<string>> *input, vector<vector<char *>> *argv, vector<cmds> *redirections)
{
    cout << ">> in format_input" << endl;

    for (int i = 0; i < input->size(); i++)
    {
        vector<char *> temp;
        for (int j = 0; j < input->at(i).size(); j++)
        {
            const char *args = input->at(i).at(j).c_str(); // reformat 2D array from string to char*
            temp.push_back((char *)args);
        }
        temp.push_back(nullptr); // nullptr terminator required for execvp execution
        argv->push_back(temp);
    }

    if (redirections->back() == is_background)
    {
        argv->pop_back(); // remove end null value due to background redirection
    }

    // error checking
    if (argv->back().front() == nullptr) // check for missing command
    {
        perror("Missing command");
        exit(EXIT_FAILURE);
    }
}

// checks for any commands
void check_input(vector<string> *parsed_args, vector<int> *redirection_indices)
{
    for (auto it = parsed_args->begin(); it != parsed_args->end(); it++) // iterate through entire vector
    {
        if (*it == "|" || *it == "$" || *it == ">" || *it == "<" || *it == ">>" || *it == "<<" || *it == "&")
        {
            redirection_indices->push_back(it - parsed_args->begin()); // if input/output redirections, pipes, and backgrounds exists, get the index at which it was found
        }
    }
}

// single command system call
int system_call(vector<string> *parsed_args)
{
    cout << ">> in system_call%" << endl;
    vector<char *> argv;
    int status;
    const char *cmd = parsed_args->front().c_str();

    // conversion from vector<string> to vector<char*>
    for (int i = 0; i < parsed_args->size(); i++)
    {
        const char *args = parsed_args->at(i).c_str();
        argv.push_back((char *)args);
    }

    argv.push_back(nullptr); // nullptr terminator required for execvp execution

    // Citation:Amir tutorial AssignExample 2 fork-execvp.cpp
    int pid = fork();
    // From now on we have two process running the same code
    if (pid < 0)
    {
        // This means fork failed
        perror("Error forking\n");
        exit(EXIT_FAILURE);
    }
    else if (pid == 0)
    {
        // This means child is running
        status = execvp(cmd, argv.data());
        perror("Error executing command in child");
        exit(EXIT_FAILURE);
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
// Citation: Amir Tutorial AssignExample2 pipe.cpp
int pipe_system_call(vector<char *> *input1, vector<char *> *input2, vector<cmds> *redirections)
{
    cout << ">> in pipe system_call%" << endl;

    int status;
    int fds[2];
    // Create a pipe
    pipe(fds);
    pid_t pid = fork();

    const char *cmd1 = input1->front(); // getting cmd for execvp
    const char *cmd2 = input2->front(); // getting cmd for execvp

    if (pid < 0)
    {
        perror("Error forking at child\n");
        exit(EXIT_FAILURE);
    }
    else if (pid == 0)
    {
        // Close the read end in the child
        close(fds[READ_END]);
        // Bind the write end with the child's output stream
        dup2(fds[WRITE_END], 1);
        // Invoke the command
        status = execvp(cmd1, input1->data());
        perror("Error executing command in child");
        exit(EXIT_FAILURE);
    }
    else
    {
        // Close the write end in the parent
        close(fds[WRITE_END]);
        // Bind the read end to parent's input stream
        dup2(fds[READ_END], 0);
        // Invoke the command
        status = execvp(cmd2, input2->data());
        perror("Error executing command in parent");
        exit(EXIT_FAILURE);
    }
    return status;
}

// overwrite system call
// Citation: Amir AssignExample2 Tutorial dup.cpp
int overwrite_system_call(vector<char *> *input1, vector<char *> *input2, vector<cmds> *redirections)
{
    cout << ">> in overwrite_system_call%" << endl;
    int status;
    int fds[2];
    // Create a pipe
    pipe(fds);
    pid_t pid = fork();

    const char *cmd1 = input1->front(); // getting cmd for execvp
    const char *cmd2 = input2->front(); // getting cmd for execvp

    if (pid < 0)
    {
        // This means fork failed
        perror("Error forking at child\n");
        exit(EXIT_FAILURE);
    }
    else if (pid == 0)
    {
        // overwrite standard output ">"
        if (redirections->front() == is_output)
        {
            if ((fds[0] = open(cmd2, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)) < 0)
            {
                perror("Output file could not be opened\n");
                exit(EXIT_FAILURE);
            }
            // Bind the write end with the child's output stream
            dup2(fds[0], 1); // 1 -> stdout
            // From now on, all my output is redirected to the file
            status = execvp(cmd1, input1->data());
            close(fds[0]);
        }

        // overwrite standard input "<"
        if (redirections->front() == is_input)
        {
            if ((fds[1] = open(cmd2, O_RDONLY | O_CREAT, S_IRUSR | S_IRGRP)) < 0)
            {
                perror("Input file could not be opened\n");
                exit(EXIT_FAILURE);
            }
            // Bind the write end with the child's output stream
            dup2(fds[1], 0); // 0 -> stdin
            // From now on, all my input is redirected to the file
            status = execvp(cmd1, input1->data());
            close(fds[1]);
        }

        perror("Error executing command in child1");
        exit(EXIT_FAILURE);
    }
    else
    {
        if (redirections->back() != is_background) // check if background execution
        {
            waitpid(pid, &status, 0);
        }
        else
        {
            cout << pid << endl;
        }
    }
    return 0;
}

// Citation: Alex Tutorial PipeDollarSignExample
int multi_system_command(vector<vector<char *>> *argv, vector<cmds> *redirections, vector<int> *redirection_indices)
{
    cout << ">> in multi_system_call%" << endl;
    int status;
    bool background;
    vector<char *> input1;
    vector<char *> input2;
    vector<char *> input3;
    vector<char *> input4;

    if (redirections->front() != my_pipe) // does not have "$"
    {
        input1 = argv->front();
        input2 = argv->back();
        if (redirections->front() == is_pipe)
        {
            status = pipe_system_call(&input1, &input2, redirections);
        }

        if (redirections->front() == is_input || redirections->front() == is_output)
        {
            status = overwrite_system_call(&input1, &input2, redirections);
        }
    }
    else // has "$"
    {
        if (redirection_indices->at(0) == 1) // cmd1 $ cmd2 cmd3
        {
            int status;
            int fds[2];
            pipe(fds);

            pid_t pid1 = fork();

            input1.push_back(argv->at(0).at(0)); // get cmd1
            input1.push_back(nullptr);

            input2.push_back(argv->at(1).at(0)); // get cmd2
            input2.push_back(nullptr);

            input3.push_back(argv->at(1).at(1)); // get cmd3
            input3.push_back(nullptr);

            if (pid1 == -1)
                perror("Error forking at child1\n");
            if (pid1 == 0) // child1
            {
                dup2(fds[READ_END], fileno(stdin));
                // anything below here will write to fileno(stdout)
                close(fds[READ_END]);
                close(fds[WRITE_END]);
                status = execvp(input3.at(0), input3.data()); // execute cmd3
                perror("Error executing command in child1");
                exit(EXIT_FAILURE);
            }
            else // parent1
            {
                pid_t pid2 = fork();

                if (pid2 == -1)
                    perror("Error forking at child2");
                if (pid2 == 0) // child2
                {
                    dup2(fds[READ_END], fileno(stdin));
                    close(fds[READ_END]);
                    close(fds[WRITE_END]);
                    status = execvp(input2.at(0), input2.data()); // execute cmd2
                    perror("Error executing command in child2");
                    exit(EXIT_FAILURE);
                }
                else // parent2
                {
                    pid_t pid3 = fork();
                    if (pid3 == -1)
                        perror("Error forking at child3");
                    if (pid3 == 0) // child3
                    {
                        // Look at this code, how do we change it for child#1 so that it redirects stdin
                        dup2(fds[WRITE_END], fileno(stdout));
                        // anything below here will write to fileno(stdout)
                        close(fds[READ_END]);
                        close(fds[WRITE_END]);

                        status = execvp(input1.at(0), input1.data()); // execute cmd1
                        perror("Error executing command in child3");
                        exit(EXIT_FAILURE);
                    }
                    else // parent3
                    {
                        // This is the very first thing that runs
                        close(fds[READ_END]);
                        close(fds[WRITE_END]);
                        waitpid(pid3, &status, 0); // wait for child3
                    }
                    waitpid(pid2, &status, 0); // wait for child2
                }
                waitpid(pid1, &status, 0); // wait for child1
            }
        }
        else if (argv->at(0).size() == 3 && argv->at(1).size() == 2) // cmd1 cmd2 $ cmd3
        {
            int fds[2];
            pipe(fds);

            pid_t pid1 = fork();

            input1.push_back(argv->at(0).at(0)); // get cmd1
            input1.push_back(nullptr);

            input2.push_back(argv->at(0).at(1)); // get cmd2
            input2.push_back(nullptr);

            input3.push_back(argv->at(1).at(0)); // get cmd3
            input3.push_back(nullptr);

            if (pid1 == -1)
                perror("Error forking at child1\n");
            if (pid1 == 0) // child1
            {
                dup2(fds[READ_END], fileno(stdin));
                // anything below here will write to fileno(stdout)
                close(fds[READ_END]);
                close(fds[WRITE_END]);
                // 2
                status = execvp(input3.at(0), input3.data()); // execute cmd3
                // 3
                perror("Error executing command in child1");
                exit(EXIT_FAILURE);
            }
            else // parent1
            {
                pid_t pid2 = fork();

                if (pid2 == -1)
                    perror("Error forking at child2");
                if (pid2 == 0)
                {
                    close(fds[READ_END]);
                    close(fds[WRITE_END]);
                    status = execvp(input2.at(0), input2.data()); // execute cmd2
                    perror("Error executing command in child2");
                    exit(EXIT_FAILURE);
                }
                else // parent2
                {
                    pid_t pid3 = fork();
                    if (pid3 == -1)
                        perror("Error forking at child3");
                    if (pid3 == 0) // child3 executes pwd
                    {
                        // Look at this code, how do we change it for child#1 so that it redirects stdin
                        dup2(fds[WRITE_END], fileno(stdout));
                        // anything below here will write to fileno(stdout)
                        close(fds[READ_END]);
                        close(fds[WRITE_END]);
                        status = execvp(input1.at(0), input1.data()); // execute cmd1
                        perror("Error executing command in child");
                        exit(EXIT_FAILURE);
                    }
                    else // parent3
                    {
                        // This is the very first thing that runs
                        int status;
                        close(fds[READ_END]);
                        close(fds[WRITE_END]);
                        waitpid(pid3, &status, 0); // wait for child3
                    }

                    int status;
                    waitpid(pid2, &status, 0);
                }
                int status;
                waitpid(pid1, &status, 0);
            }
        }
        else if (argv->at(0).size() == 3 && argv->at(1).size() == 3) // cmd1 cmd2 $ cmd3 cmd4
        {
            int fds[2];
            pipe(fds);

            pid_t pid1 = fork();

            input1.push_back(argv->at(0).at(0)); // get cmd1
            input1.push_back(nullptr);

            input2.push_back(argv->at(0).at(1)); // get cmd2
            input2.push_back(nullptr);

            input3.push_back(argv->at(1).at(0)); // get cmd3
            input3.push_back(nullptr);

            input4.push_back(argv->at(1).at(1)); // get cmd4
            input4.push_back(nullptr);

            if (pid1 == -1)
                perror("Error forking at child1\n");
            if (pid1 == 0) // child1
            {
                dup2(fds[READ_END], fileno(stdin));
                // anything below here will write to fileno(stdout)
                close(fds[READ_END]);
                close(fds[WRITE_END]);
                status = execvp(input4.at(0), input4.data());
                perror("Error executing command in child1");
                exit(EXIT_FAILURE);
            }
            else // parent1
            {
                pid_t pid2 = fork();

                if (pid2 == -1)
                    perror("Error forking at child2");
                if (pid2 == 0) // child2
                {
                    dup2(fds[READ_END], fileno(stdin));
                    // anything below here will write to fileno(stdout)
                    close(fds[READ_END]);
                    close(fds[WRITE_END]);
                    status = execvp(input3.at(0), input3.data());
                    perror("Error executing command in child2");
                    exit(EXIT_FAILURE);
                }
                else // parent2
                {
                    pid_t pid3 = fork();
                    if (pid3 == -1)
                        perror("Error forking at child3");
                    if (pid3 == 0) // child3
                    {
                        dup2(fds[WRITE_END], fileno(stdout));
                        close(fds[READ_END]);
                        close(fds[WRITE_END]);
                        status = execvp(input2.at(0), input2.data());
                        perror("Error executing command in child3");
                        exit(EXIT_FAILURE);
                    }
                    else // parent3
                    {
                        pid_t pid4 = fork();
                        if (pid4 == -1)
                            perror("Error forking at child4");
                        if (pid4 == 0) // child4
                        {
                            dup2(fds[WRITE_END], fileno(stdout));
                            // anything below here will write to fileno(stdout)
                            close(fds[READ_END]);
                            close(fds[WRITE_END]);
                            status = execvp(input1.at(0), input1.data());
                            perror("Error executing command in child4");
                            exit(EXIT_FAILURE);
                        }
                        else // parent4
                        {
                            // This is the very first thing that runs
                            int status;
                            close(fds[READ_END]);
                            close(fds[WRITE_END]);
                            waitpid(pid4, &status, 0); // wait for child4
                        }
                        int status;
                        waitpid(pid3, &status, 0); // wait for child3
                    }
                    int status;
                    waitpid(pid2, &status, 0);
                }
                int status;
                waitpid(pid1, &status, 0);
            }
        }
    }
    return status;
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
                multi_cmd(&parsed_args, &redirection_indices, &inputs, &redirections); // sort commands and arguments
                format_input(&inputs, &argv, &redirections);                           // reformat input for execvp execution and error checking

                /*
                for (auto it = redirections.begin(); it != redirections.end(); it++)
                {
                    char * temp = argv.at(it - redirections.begin()).front();
                    if (*it == is_output)
                    {
                        if (temp == "file.txt")
                            perror("testing");
                    }
                }
                */
                status = multi_system_command(&argv, &redirections, &redirection_indices); // execute commands
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