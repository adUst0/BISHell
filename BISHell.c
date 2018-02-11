#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/wait.h>
#include "BISHell.h"

// Array of strings to store built-in commads.
char *builtinNames[] = {
    "cd",
    "chdir",
    "exit",
    "q",
    "quit"
};

// Array of pointers to handle the built-in commads.
BuiltinFunc builtinFunc[] = {
    &shChdir,
    &shChdir,
    &shExit,
    &shExit,
    &shExit
};

int builtinLen(void)
{
    return sizeof(builtinNames) / sizeof(char*);
}

int shChdir(char **arg_v)
{
    if (!arg_v[1])
    {
        if (HOME)
        {
            chdir(HOME);
        }
        else
        {
            write(STDOUT_FILENO, "cd: Missing argument\n", strlen("cd: Missing argument\n"));
        }
    }
    else
    {
        if (chdir(arg_v[1]) != 0)
        {
            return EXIT_FAILURE;
        }
        else 
        {
            return EXIT_SUCCESS;
        }
    }
}

int shExit(char **arg_v) 
{
    exit(EXIT_SUCCESS);
}

void shInit(void) 
{
    if (HOME)
    {
        chdir(HOME);
    }
}

void shLoop(void)
{
    while(1)
    {
        command *cmd = malloc(sizeof(command));
        commandInit(cmd);

        write(STDOUT_FILENO, PROMPT, strlen(PROMPT));
        
        shReadLine(cmd);
        shParseLine(cmd);
        shExecuteLine(cmd);

        commandDel(cmd);
        free(cmd);
    }
}

void shTerminate() 
{
    exit(EXIT_SUCCESS);
}

void shReadLine(command *cmd) 
{
    cmd->buff = malloc(BUFF_LENGTH);
    int buffCapacity = BUFF_LENGTH;
    int buffSize = 0;

    assertAlloc(cmd->buff);

    char byte;
    int readStatus;

    while(1)
    {
        readStatus = read(STDIN_FILENO, &byte, 1);
        switch(readStatus)
        {
            case -1:
                perror("I/O error");
                exit(EXIT_FAILURE);
            case 0: 
                cmd->buff[buffSize] = '\0';
                return;
            default:
                if(byte == '\n')
                {
                    cmd->buff[buffSize] = '\0';
                    return;
                }
                else
                {
                    cmd->buff[buffSize++] = byte;
                }
        }

        if (buffSize >= buffCapacity)
        {
            resizeArr(cmd->buff, buffCapacity);
        }
    }
}

void shParseLine(command *cmd) 
{
    int capacity = ARGUMENTS;
    int size = 0;
    cmd->arguments = malloc(sizeof(char*) * capacity);

    if (cmd->buff[0] == '\0')
    {
        cmd->arguments[0] = NULL;
        return;
    }

    // Current argument
    char *arg = malloc(BUFF_LENGTH);
    int argCapacity = BUFF_LENGTH;
    int argSize = 0;

    assertAlloc(cmd->arguments);
    assertAlloc(arg);

    for(int buffPos = 0; cmd->buff[buffPos] != '\0'; buffPos++)
    {
        char c = cmd->buff[buffPos];

        if (c == ' ' && argSize != 0)
        {
            arg[argSize] = '\0';
            cmd->arguments[size++] = arg;
            argSize = 0;
            arg = malloc(BUFF_LENGTH);
            assertAlloc(arg);
        }
        else if(c != ' ')
        {
            arg[argSize++] = c;
        }

        if (argSize >= argCapacity)
        {
            resizeArr(arg, argCapacity);
        }
        if (size >= capacity)
        {
            resizeArr(cmd->arguments, capacity);
        }
    }

    // If currently 0 non-whitespace symbols are read, don't add new empty argument to cmd->arguments
    if (argSize != 0)
    {
        arg[argSize] = '\0';
        cmd->arguments[size++] = arg;
    }
    else /*if (argSize == 0)*/
    {
        free(arg);
    }

    cmd->arguments[size] = NULL;
}

int shExecuteLine(command *cmd)
{
    int cmdOffset = 0;

    for(int i = 0; cmd->arguments[i] != NULL; i++)
    {
        if (strchr(cmd->arguments[i], '&'))
        {
            cmd->background = 1;
            char * tmp = cmd->arguments[i];
            cmd->arguments[i] = NULL;

            shExecute(cmd, cmdOffset);

            cmd->arguments[i] = tmp;
            cmdOffset = i+1;
            cmd->background = 0;
        }
        else if (strchr(cmd->arguments[i], '>'))
        {
            if(cmd->arguments[i+1] == NULL)
            {
                write(STDERR_FILENO, "Missing file name for I/O redirection.\n", strlen("Missing file name for I/O redirection.\n"));
                return -1;
            }
            int append = (strchr(cmd->arguments[i], '>')[1] == '>') ? O_APPEND : O_TRUNC;
            commandAddRedirection(cmd, 1, cmd->arguments[i+1], O_WRONLY | O_CREAT | append);
            free(cmd->arguments[i]);
            free(cmd->arguments[i+1]);
            int j = i;
            for (j = i; cmd->arguments[i+2] != NULL; j++)
            {
                cmd->arguments[j] = cmd->arguments[j+2];
            }
            cmd->arguments[j] = NULL;
            i--;
        }
        else if (strchr(cmd->arguments[i], '<'))
        {
            if(cmd->arguments[i+1] == NULL)
            {
                write(STDERR_FILENO, "Missing file name for I/O redirection.\n", strlen("Missing file name for I/O redirection.\n"));
                return -1;
            }
            commandAddRedirection(cmd, 0, cmd->arguments[i+1], O_RDONLY);
            free(cmd->arguments[i]);
            free(cmd->arguments[i+1]);
            int j;
            for (j = i; cmd->arguments[i+2] != NULL; j++)
            {
                cmd->arguments[j] = cmd->arguments[j+2];
            }
            cmd->arguments[j] = NULL;
            i--;
        }
    }

    shExecute(cmd, cmdOffset);
}

int shExecute(command *cmd, int offset) 
{
    int status = 0;
    char **arguments = cmd->arguments + offset;

    if (arguments[0] == NULL)
    {
        return 0;
    }

    for(int i = 0; i < builtinLen(); i++)
    {
        if (strcmp(builtinNames[i], arguments[0]) == 0)
        {
            status = (*builtinFunc[i])(arguments);
            return status;
        }
    }

    int pid = fork();
    switch(pid)
    {
        case 0: // child
            commandOpenFiles(cmd);
            if (execvp(arguments[0], arguments) == -1) 
            {
                perror("Can't create process");
                exit(EXIT_FAILURE);
            }
            break;
        case -1:
            perror("Can't fork");
            exit(EXIT_FAILURE);
            break;
        default: // father
            if (!cmd->background) 
            {
                waitpid(pid, &status, 0);
            }
            else
            {
                printf("Background process created with PID: %d\n", pid);
            }
            break;
    }

    return status;  
}

int main()
{
    shInit();
    shLoop();
    shTerminate();
    return EXIT_SUCCESS;
}