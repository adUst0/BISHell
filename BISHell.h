#ifndef BISHELL_H
#define BISHELL_H

#define BUFF_LENGTH 512
#define ARGUMENTS 64
#define STD_IN 0
#define STD_OUT 1
#define PROMPT "-> "

typedef int (*BuiltinFunc)(char**);

int shCd(char **arg_v);
int shExit(char **arg_v); 

void shInit(void);
void shLoop(void);
void shTerminate(void);

char* shReadLine(void);
char** shParseLine(char* buff);
int shExecute(char **arg_v);

#include "BISHell.c"
#endif //BISHELL_H