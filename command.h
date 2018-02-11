#ifndef COMMAND_H
#define COMMAND_H

#include <unistd.h>

typedef struct
{
    char** arguments;
    char* buff;
    char *fdi, *fdo, *fderr;
    int flagsi, flagso, flagserr;
    char background;
}command;

void commandInit(command *cmd)
{
    cmd->arguments = NULL;
    cmd->buff = NULL;
    cmd->fdi = NULL;
    cmd->fdo = NULL;
    cmd->fderr = NULL;
    cmd->flagsi = 0;
    cmd->flagso = 0;
    cmd->flagserr = 0;
    cmd->background = 0;
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
    
    if (cmd->fdi)
    {
        free(cmd->fdi);
    }
    if (cmd->fdo)
    {
        free(cmd->fdo);
    }
    if (cmd->fderr)
    {
        free(cmd->fderr);
    }
}

void commandAddRedirection(command *cmd, int channel, char *fname, int flags)
{
    switch(channel)
    {
        case 0:
            if (cmd->fdi) free(cmd->fdi);
            cmd->fdi = malloc(strlen(fname) + 1);
            strcpy(cmd->fdi, fname);
            cmd->flagsi = flags;
            break;
        case 1:
            if (cmd->fdo) free(cmd->fdo);
            cmd->fdo = malloc(strlen(fname) + 1);
            strcpy(cmd->fdo, fname);
            cmd->flagso = flags;
            break;
        case 2:
            if (cmd->fderr) free(cmd->fderr);
            cmd->fderr = malloc(strlen(fname) + 1);
            strcpy(cmd->fderr, fname);
            cmd->flagserr = flags;
            break;
    }
}

void commandOpenFiles(command *cmd)
{
    if(cmd->fdi)
    {
        close(STDIN_FILENO);
        if (open(cmd->fdi, cmd->flagsi) == -1)
        {
            perror("Can't open file");
            exit(EXIT_FAILURE);
        }
    }
    if(cmd->fdo)
    {
        close(STDOUT_FILENO);
        if (open(cmd->fdo, cmd->flagso, 0666) == -1)
        {
            perror("Can't open file");
            exit(EXIT_FAILURE);
        }
    }
    if(cmd->fderr)
    {
        close(STDIN_FILENO);
        if (open(cmd->fderr, cmd->flagserr) == -1)
        {
            perror("Can't open file");
            exit(EXIT_FAILURE);
        }
    }
}

#endif //COMMAND_H