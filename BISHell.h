#ifndef BISHELL_H
#define BISHELL_H

#include <unistd.h>
#include <stdlib.h>
#include "command.h"

#define BUFF_LENGTH 512
#define ARGUMENTS   64

#define PROMPT "-> "
#define HOME getenv("HOME")

// The program terminates with EXIT_FAILURE if there was a problem allocating memory.
#define assertAlloc(ptr)                                                \
    if(!ptr)                                                            \
    {                                                                   \
        write(2, "Allocation error!\n", strlen("Allocation error!\n")); \
        exit(EXIT_FAILURE);                                             \
    }

// Doubles the value of capacity and reallocates new memory for the ptr.
#define resizeArr(ptr, capacity)                                        \
    capacity *= 2;                                                      \
    ptr = realloc(ptr, sizeof(*ptr) * capacity);                        \
    assertAlloc(cmd->buff);

typedef int (*BuiltinFunc)(char**);

int shChdir(char **arg_v);
int shExit(char **arg_v); 
int shExecute(command *cmd, int offset);

void shInit(void);
void shLoop(void);
void shTerminate(void);

void shReadLine(command *cmd);
void shParseLine(command *cmd);
int  shExecuteLine(command *cmd);

#endif //BISHELL_H