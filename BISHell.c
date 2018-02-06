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
            write(STD_OUT, "cd: Missing argument\n", strlen("cd: Missing argument\n"));
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
        command cmd = commandInit();

        write(STD_OUT, PROMPT, strlen(PROMPT));
        shReadLine(&cmd);
        shParseLine(&cmd);

        commandDel(&cmd);
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
        readStatus = read(STD_IN, &byte, 1);
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
        switch(c)
        {
            case ' ':
                // Skip multiple whitespaces
                if (argSize == 0)
                {
                    continue;
                }
                arg[argSize] = '\0';
                cmd->arguments[size++] = arg;
                arg = malloc(BUFF_LENGTH);
                argSize = 0;
                assertAlloc(arg);
                break;
            case '&':
                cmd->background = 1;
                if (argSize != 0)
                {
                    arg[argSize] = '\0';
                    cmd->arguments[size++] = arg;
                }
                else
                {
                    free(arg);
                }
                cmd->arguments[size] = NULL;
                shExecute(cmd);
                cmd->background = 0;
                for(int j = 0; j < size; j++) 
                    free(cmd->arguments[j]);
                size = 0;
                argCapacity = BUFF_LENGTH;
                argSize = 0;
                arg = malloc(argCapacity);
                assertAlloc(arg);
                break;
            default:
                arg[argSize++] = c;
                break;
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
        cmd->arguments[size] = NULL;
    }
    else
    {
        cmd->arguments[size] = NULL;
        free(arg);
    }

    shExecute(cmd);
}

int shExecute(command *cmd) 
{
    int status = 0;

    if (!cmd->arguments[0])
    {
        return 0;
    }

    for(int i = 0; i < builtinLen(); i++)
    {
        if (strcmp(builtinNames[i], cmd->arguments[0]) == 0)
        {
            status = (*builtinFunc[i])(cmd->arguments);
            return status;
        }
    }

    int pid = fork();
    switch(pid)
    {
        case 0: // child
            dup2(cmd->fdi, STD_IN);
            dup2(cmd->fdo, STD_OUT);
            dup2(cmd->fderr, STD_ERR);
            if (execvp(cmd->arguments[0], cmd->arguments) == -1) 
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