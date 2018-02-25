#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/wait.h>
#include "BISHell.h"
#include "Utils.h"

void shInit(void)
{
    if (HOME)
    {
        chdir(HOME);
    }
}

void shTerminate() 
{
    exit(EXIT_SUCCESS);
}

void shLoop(void)
{
    while(1)
    {
        Command *cmd = CommandInit();

        write(STDOUT_FILENO, PROMPT, strlen(PROMPT));
        shReadLine(cmd);
        shParseLine(cmd);
        shExecuteLine(cmd);

        CommandDel(cmd);
    }
}

void shReadLine(Command *cmd) 
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

void shParseLine(Command *cmd) 
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
    cmd->argumentsCount = size;
}

int shExecuteLine(Command *cmd)
{
    int first, last;

    for(int i = 0; cmd->arguments[i] != NULL; i++)
    {
        if (strchr(cmd->arguments[i], '&'))
        { 
            cmd->background = 1;
            CommandPartialExecute(cmd, 0, i);

            first = 0, last = i + 1;
            CommandEraseArguments(cmd, first, last);
            i -= (last - first);

            cmd->background = 0;
            CommandResetRedirections(cmd);
        }
        else if (strchr(cmd->arguments[i], '|'))
        {
        	cmd->pipe = 1;
            pipe(cmd->pipefd);
            cmd->fdo = cmd->pipefd[1]; // cmd will write to the pipe

            CommandPartialExecute(cmd, 0, i);
            first = 0, last = i + 1;
            CommandEraseArguments(cmd, first, last);
            i -= (last - first);

            close (cmd->pipefd[1]);
            cmd->fdi = cmd->pipefd[0]; // next Command will read from the current pipe
            cmd->fdo = 1;
        }
        else if (strchr(cmd->arguments[i], '>'))
        {
            if(cmd->arguments[i+1] == NULL)
            {
                write(STDERR_FILENO, "Missing file name for I/O redirection.\n", strlen("Missing file name for I/O redirection.\n"));
                return -1;
            }
            int append = (strchr(cmd->arguments[i], '>')[1] == '>') ? O_APPEND : O_TRUNC;
            CommandAddRedirection(cmd, STDOUT_FILENO, cmd->arguments[i+1], O_WRONLY | O_CREAT | append);
            
            first = i, last = i + 2;
            CommandEraseArguments(cmd, first, last);
            i -= (last - first);
        }
        else if (strchr(cmd->arguments[i], '<'))
        {
            if(cmd->arguments[i+1] == NULL)
            {
                write(STDERR_FILENO, "Missing file name for I/O redirection.\n", strlen("Missing file name for I/O redirection.\n"));
                return -1;
            }
            CommandAddRedirection(cmd, STDIN_FILENO, cmd->arguments[i+1], O_RDONLY);
            first = i, last = i + 2;
            CommandEraseArguments(cmd, first, last);
            i -= (last - first);
        }
    }

    return CommandExecute(cmd); // execute the last command
}

int main()
{
    shInit();
    shLoop();
    shTerminate();
    return EXIT_SUCCESS;
}