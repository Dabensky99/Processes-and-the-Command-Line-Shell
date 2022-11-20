#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <sys/wait.h>

void start_command(char *words[], int nwords)
{
    char *native_arr[100];
    int native_cmds;
    pid_t pid;

    for (native_cmds = 0; native_cmds < nwords; native_cmds++)
    {
        native_arr[native_cmds] = words[native_cmds + 1];
    }
    native_arr[nwords] = NULL;

    pid = fork();
    if (pid == 0) // child process
    {
        printf("myshell: process %d started\n", getpid());
        execvp(native_arr[0], native_arr); // execute a command
    }
    else if (pid < 0) // parent process
    {
        perror("fork failed\n");
        exit(1);
    }
}

void wait_command()
{

    int status;
    pid_t pid = wait(&status);

    // check if there are any running processes
    if (pid < 0)
    {
        if (errno == ECHILD)
        {
            printf("myshell: no processes left\n");
        }
    }
    else if (WIFEXITED(status)) // check if the shell exited normally or abnormally
    {
        int result = WEXITSTATUS(status);
        if (result == 0)
        {
            printf("myshell: process %d exited normally with status %d\n", pid, result);
        }
        else
        {
            printf("myshell: process %d exited abnormally with status %d\n", pid, result);
        }
    }
    else if (WIFSIGNALED(status))
    {
        int signal = WTERMSIG(status); // Check which signal specifically ended the process
        printf("myshell: process %d exited abnormally with signal %d: %s\n", pid, signal, strsignal(signal));
    }
}

void run_command(char *words[], int nwords)
{
    char *native_arr[100];
    int native_cmds;
    pid_t pid;
    pid_t wait_process;
    int status;

    // set parameters to every word after command
    for (native_cmds = 0; native_cmds < nwords; native_cmds++)
    {
        native_arr[native_cmds] = words[native_cmds + 1];
    }
    native_arr[nwords] = NULL;

    pid = fork();
    if (pid == 0) // child process
    {
        execvp(native_arr[0], native_arr);
    }
    else if (pid > 0) // parent process
    {
        wait_process = waitpid(pid, &status, 0);
        if (wait_process == -1)
        {
            perror("myshell: an error occured\n");
            exit(-1);
        }
        else
        {
            if (WIFEXITED(status))
            {
                // check if the process ended normally or abnormally.
                int result = WEXITSTATUS(status);
                if (result == 0)
                {
                    printf("myshell: process %d exited normally with status %d\n", pid, result);
                }
                else
                {
                    printf("myshell: process %d exited abnormally with status %d\n", pid, result);
                }
            }
        }
    }
    else
    {
        perror("fork failed\n");
        exit(1);
    }
}

void kill_command(char *words[])
{
    // set kill, stop, continue signals for respective commands
    if (strcmp(words[0], "kill") == 0)
    {
        if (kill(atoi(words[1]), SIGKILL) < 0)
        {
            printf("myshell: %s\n", strerror(errno));
        }
        else
        {
            printf("myshell: process %d killed\n", atoi(words[1]));
        }
    }
    else if (strcmp(words[0], "stop") == 0)
    {
        if (kill(atoi(words[1]), SIGSTOP) < 0)
        {
            printf("myshell: %s\n", strerror(errno));
        }
        else
        {
            printf("myshell: process %d stopped\n", atoi(words[1]));
        }
    }
    else if (strcmp(words[0], "continue") == 0)
    {
        if (kill(atoi(words[1]), SIGCONT) < 0)
        {
            printf("myshell: %s\n", strerror(errno));
        }
        else
        {
            printf("myshell: process %d continued\n", atoi(words[1]));
        }
    }
}

int main(int argc, char **argv)
{
    char line[4096];
    char command[100];
    char *words[100];
    char *token;
    int nwords;

    while (1)
    {
        // Restart the loop with empty variable
        nwords = 0;
        words[0] = "";

        // start the shell
        printf("myshell> ");
        fflush(stdout);
        fgets(line, 4096, stdin);

        // Get the first word and tokenize the words
        token = strtok(line, " \n");
        while (token != NULL)
        {
            words[nwords] = strdup(token);
            token = strtok(0, " \n");
            nwords++;
        }
        nwords++;
        words[nwords] = 0;

        // Shift the first inputted word to the command.
        strcpy(command, words[0]);

        // Check if a command was used. If so, run the function.
        if (strcmp(command, "start") == 0)
        {
            start_command(words, nwords);
        }
        else if (strcmp(command, "run") == 0)
        {
            run_command(words, nwords);
        }
        else if (strcmp(command, "wait") == 0)
        {
            wait_command();
        }
        else if ((strcmp(command, "kill") == 0))
        {
            kill_command(words);
        }
        else if ((strcmp(command, "stop") == 0))
        {
            kill_command(words);
        }
        else if ((strcmp(command, "continue") == 0))
        {
            kill_command(words);
        }
        else if ((strcmp(command, "exit") == 0))
        {
            break;
        }
        else if (strcmp(command, "quit") == 0)
        {
            break;
        }
        else
        {
            printf("myshell: unknown command: %s\n", command);
        }
    }
    exit(0);
}