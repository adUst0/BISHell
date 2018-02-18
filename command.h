#ifndef COMMAND_H
#define COMMAND_H

#include <unistd.h>

typedef struct
{
    char** arguments; // array to store arguments
    char* buff; // buffer to store line from stdin
    char *inFileName; // Input redirection file name
    char *outFileName; // Output redirection file name
    char *errFileName; // Stderr redirection file name
    int inFlags; // Input redirection file flags 
    int outFlags; // Output redirection file flags 
    int errFlags; // Stderr redirection file flags 
    char background; // 1 if current command is in background mode
    char pipe; // 1 if the current command uses a pipe
    int pipefd[2]; // keep fds for the pipe
    int fdi; // file descriptors for standart I/O
    int fdo; // file descriptors for standart I/O
    int fde; // file descriptors for standart I/O
} command;

void commandInit(command *cmd)
{
    cmd->arguments = NULL;
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
}

void commandDel(command *cmd)
{
    if(cmd->buff)
    {
        free(cmd->buff);
    }
    if(cmd->arguments)
    {
        for(int i = 0; cmd->arguments[i]; i++)
        {
            free(cmd->arguments[i]);
        }
        free(cmd->arguments);
    }
    
    if (cmd->inFileName)
    {
        free(cmd->inFileName);
    }
    if (cmd->outFileName)
    {
        free(cmd->outFileName);
    }
    if (cmd->errFileName)
    {
        free(cmd->errFileName);
    }
}

void commandAddRedirection(command *cmd, int channel, char *fname, int flags)
{
    switch(channel)
    {
        case 0:
            if (cmd->inFileName) free(cmd->inFileName);
            cmd->inFileName = malloc(strlen(fname) + 1);
            strcpy(cmd->inFileName, fname);
            cmd->inFlags = flags;
            break;
        case 1:
            if (cmd->outFileName) free(cmd->outFileName);
            cmd->outFileName = malloc(strlen(fname) + 1);
            strcpy(cmd->outFileName, fname);
            cmd->outFlags = flags;
            break;
        case 2:
            if (cmd->errFileName) free(cmd->errFileName);
            cmd->errFileName = malloc(strlen(fname) + 1);
            strcpy(cmd->errFileName, fname);
            cmd->errFlags = flags;
            break;
    }
}

void commandOpenFiles(command *cmd)
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

void commandHandlePipe(command *cmd)
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

#endif //COMMAND_H