#include "Utils.h"
#include "Command.h"
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/wait.h>

int stdinfd, stdoutfd, stderrfd;

Command* CommandInit()
{
	Command *cmd = malloc(sizeof(Command));
    assertAlloc(cmd);
    cmd->arguments = NULL;
    cmd->argumentsCount = 0;
    cmd->buff = NULL;
    cmd->inFileName = NULL;
    cmd->outFileName = NULL;
    cmd->errFileName = NULL;
    cmd->inFlags = 0;
    cmd->outFlags = 0;
    cmd->errFlags = 0;
    cmd->background = 0;
    cmd->pipe = 0;
    cmd->pipefd[0] = cmd->pipefd[1] = -1;
    cmd->fdi = STDIN_FILENO;
    cmd->fdo = STDOUT_FILENO;
    cmd->fde = STDERR_FILENO;

    dup2(STDIN_FILENO, stdinfd);
    dup2(STDOUT_FILENO, stdoutfd);
    dup2(STDERR_FILENO, stderrfd);

    return cmd;
}

void CommandDel(Command *cmd)
{
    if(cmd->buff)
    {
        delMemory(cmd->buff);
    }
    if(cmd->arguments)
    {
        for(int i = 0; i < cmd->argumentsCount; i++)
        {
            delMemory(cmd->arguments[i]);
        }
        delMemory(cmd->arguments);
    }
    
    if (cmd->inFileName)
    {
        delMemory(cmd->inFileName);
    }
    if (cmd->outFileName)
    {
        delMemory(cmd->outFileName);
    }
    if (cmd->errFileName)
    {
        delMemory(cmd->errFileName);
    }

    delMemory(cmd);
}

void CommandAddRedirection(Command *cmd, int channel, char *fname, int flags)
{
    switch(channel)
    {
        case STDIN_FILENO:
            if (cmd->inFileName) delMemory(cmd->inFileName);
            cmd->inFileName = malloc(strlen(fname) + 1);
            assertAlloc(cmd->inFileName);
            strcpy(cmd->inFileName, fname);
            cmd->inFlags = flags;
            break;
        case STDOUT_FILENO:
            if (cmd->outFileName) delMemory(cmd->outFileName);
            cmd->outFileName = malloc(strlen(fname) + 1);
            assertAlloc(cmd->outFileName);
            strcpy(cmd->outFileName, fname);
            cmd->outFlags = flags;
            break;
        case STDERR_FILENO:
            if (cmd->errFileName) delMemory(cmd->errFileName);
            cmd->errFileName = malloc(strlen(fname) + 1);
            assertAlloc(cmd->errFileName);
            strcpy(cmd->errFileName, fname);
            cmd->errFlags = flags;
            break;
    }
}

void CommandResetRedirections(Command *cmd)
{
    if (cmd->inFileName) 
    {
        delMemory(cmd->inFileName);
    }
    if (cmd->outFileName) 
    {
        delMemory(cmd->outFileName);
    }
    if (cmd->errFileName) 
    {
        delMemory(cmd->errFileName);
    }

    dup2(stdinfd, STDIN_FILENO);
    dup2(stdoutfd, STDOUT_FILENO);
    dup2(stderrfd, STDERR_FILENO);
}

void CommandOpenFiles(Command *cmd)
{
    if(cmd->inFileName)
    {
        close(STDIN_FILENO);
        if (open(cmd->inFileName, cmd->inFlags) == -1)
        {
            perror("Can't open file");
            exit(EXIT_FAILURE);
        }
    }
    if(cmd->outFileName)
    {
        close(STDOUT_FILENO);
        if (open(cmd->outFileName, cmd->outFlags, 0666) == -1)
        {
            perror("Can't open file");
            exit(EXIT_FAILURE);
        }
    }
    if(cmd->errFileName)
    {
        close(STDIN_FILENO);
        if (open(cmd->errFileName, cmd->errFlags) == -1)
        {
            perror("Can't open file");
            exit(EXIT_FAILURE);
        }
    }
}

void CommandHandlePipe(Command *cmd)
{
    if (cmd->pipe && (cmd->fdi != 0 || cmd->fdo != 1))
    {
        if (cmd->fdi != 0)
        {
            dup2 (cmd->fdi, 0);
            close (cmd->fdi);
        }
        if (cmd->fdo != 1)
        {                  
            dup2 (cmd->fdo, 1);
            close (cmd->fdo);
        }
    }
}

int CommandExecute(Command *cmd)
{
    int status = 0;

    if (cmd->arguments[0] == NULL)
    {
        return 0;
    }

    if (!strcmp(cmd->arguments[0], "cd") || !strcmp(cmd->arguments[0], "chdir"))
    {
    	return CommandChdir(cmd);
    }
    else if(!strcmp(cmd->arguments[0], "exit") || !strcmp(cmd->arguments[0], "quit") || !strcmp(cmd->arguments[0], "q"))
    {
    	return CommandExit(cmd);
    }

    int pid = fork();
    switch(pid)
    {
        case 0: // child
            CommandOpenFiles(cmd);
            CommandHandlePipe(cmd);
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

int CommandPartialExecute(Command *cmd, int first, int last)
{
    int status;
    char *tmp = cmd->arguments[last];
    cmd->arguments[last] = NULL;
    status = CommandExecute(cmd);
    cmd->arguments[last] = tmp;
    return status;
}


int CommandEraseArguments(Command *cmd, int first, int last)
{
	if (first < 0 || last > cmd->argumentsCount)
	{
		return -1;
	}

	int delta = last - first;

	for(int i = first; i < cmd->argumentsCount; i++)
	{
        if(cmd->arguments[i])
        {
            delMemory(cmd->arguments[i]);
        }
        if (i < cmd->argumentsCount - delta)
        {
            cmd->arguments[i] = cmd->arguments[i + delta];
            cmd->arguments[i + delta] = NULL;
        }
	}

    cmd->argumentsCount = cmd->argumentsCount - delta;
	cmd->arguments[cmd->argumentsCount] = NULL;

	return EXIT_SUCCESS;
}

// Builtin Commands: Exit
int CommandExit(Command *cmd)
{
	exit(EXIT_SUCCESS);
}

// Builtin Commands: Chdir
int CommandChdir(Command *cmd)
{
    int status = EXIT_FAILURE;
    if (!cmd->arguments[1]) 
    {
        if (HOME)
        {
            chdir(HOME);
            status = EXIT_SUCCESS;
        }
        else
        {
            write(STDOUT_FILENO, "cd: Missing argument\n", strlen("cd: Missing argument\n"));
        }
    }
    else
    {
        if (chdir(cmd->arguments[1]) != 0)
        {
            status = EXIT_FAILURE;
        }
        else 
        {
            status = EXIT_SUCCESS;
        }
    }
    return status;
}

void CommandPrintArguments(Command *cmd)
{	
	for(int i = 0; i < cmd->argumentsCount; i++)
	{
		printf("%s ", cmd->arguments[i]);
	}
    printf("\n");
}