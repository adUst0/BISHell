#ifndef BISHELL_H
#define BISHELL_H

#include <unistd.h>
#include <stdlib.h>

#define BUFF_LENGTH 512
#define ARGUMENTS 	64
#define STD_IN 		0
#define STD_OUT 	1
#define STD_ERR 	2
#define PROMPT "-> "

typedef struct
{
	char** arguments;
	char* buff;
	char background;
	int fdi, fdo, fderr;
}command;

command commandInit()
{
	command cmd;
	cmd.arguments = NULL;
	cmd.buff = NULL;
	cmd.background = 0;
	cmd.fdi = STD_IN;
	cmd.fdo = STD_OUT;
	cmd.fderr = STD_ERR;
	return cmd;
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
}

typedef int (*BuiltinFunc)(char**);

int shCd(char **arg_v);
int shExit(char **arg_v); 

void shInit(void);
void shLoop(void);
void shTerminate(void);

void shReadLine(command *cmd);
void shParseLine(command *cmd);
int shExecute(command *cmd);

#endif //BISHELL_H