#ifndef BISHELL_H
#define BISHELL_H

#include "Command.h"

#define BUFF_LENGTH 512
#define ARGUMENTS   64
#define MAX_HISTORY 32

#define PROMPT "-> "

void shInit(void);
void shLoop(void);
void shTerminate(void);

void shReadLine(Command *cmd);
void shParseLine(Command *cmd);
int  shExecuteLine(Command *cmd);

#endif //BISHELL_H