#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/wait.h>

static char* HOME = NULL;

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
	&shCd,
	&shCd,
	&shExit,
	&shExit,
	&shExit
};

/**
 * Return the number of built-in functions described in builtinNames[].
 *
 * @return 	the number of built-in functions
 */
int builtinLen(void)
{
	return sizeof(builtinNames) / sizeof(char*);
}

/**
 * Change the working directory to the path described in arg_v[1].
 * If no path is specified and HOME is set, the working directory is set to HOME.
 *
 * @param 	char** arg_v 	array of argumеnts containing the command cd and the desired path
 * @return 	0 on success, -1 otherwise
 */
int shCd(char **arg_v)
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

/**
 * Stop the execution of the current proccess. 
 */
int shExit(char **arg_v) 
{
	exit(EXIT_SUCCESS);
}

/**
 * Initialization of the shell.
 * If found, set the HOME variable. 
 */
void shInit(void) 
{
	HOME = getenv("HOME");
	if (HOME)
	{
		chdir(HOME);
	}
}

/**
 * Main loop of the Shell.
 * Read new line from the standart input.
 * Split the line into arguments and execute them.
 */
void shLoop(void)
{
	while(1)
	{
		char **arg_v = NULL;
		char *buff = NULL;
		int status;

		write(1, PROMPT, strlen(PROMPT));
		buff = shReadLine();
		arg_v = shParseLine(buff);
		status = shExecute(arg_v);

		free(buff);
		free(arg_v);
	}
}

/**
 * Stop the execution of the current proccess.
 */
void shTerminate() 
{
	exit(EXIT_SUCCESS);
}

/**
 * Read input in a buffer. 
 * If 0 non-whitespace symbols are read, NULL is returned.
 *
 * @return 	line read from standard input or NULL if only white spaces or 0 characters are read.
 */
char* shReadLine() 
{
	char *buff = malloc(BUFF_LENGTH);
	int buffSize = BUFF_LENGTH;
	int i = 0;

	if (!buff)
	{
		perror("Allocation error");
		exit(EXIT_FAILURE);
	}

	char byte[1];
	int readStatus = read(STD_IN, byte, 1);

	while(1)
	{

		if (readStatus == -1)
		{
			perror("I/O error");
			exit(EXIT_FAILURE);
		}
		else if (readStatus == 0 || byte[0] == '\n')
		{
			if (i == 0)
			{
				return NULL;
			}
			else 
			{	
				buff[i] = '\0';
				return buff;
			}
		}
		else 
		{
			buff[i++] = byte[0];
		}

		if (i >= buffSize)
		{
			buffSize *= 2;
			buff = realloc(buff, buffSize);
			if (!buff)
			{
				perror("Allocation error");
				exit(EXIT_FAILURE);
			}
		}

		readStatus = read(STD_IN, byte, 1);
	}
}

/**
 * Split the buffer into command and arguments. 
 * If the buffer is NULL pointer, NULL is returned. 
 *  
 * @param 	char *buff 	containing line read from standard input
 * @return 	array of command and arguments or NULL if there is no command specified
 */
char** shParseLine(char* buff) 
{
	if (!buff)
	{
		return NULL;
	}

	// Array that will store the arguments
	char **arg_v = malloc(sizeof(char*) * ARGUMENTS);
	int arg_vSize = ARGUMENTS;
	int arg_vPos = 0;

	// Current argument
	char *arg = malloc(BUFF_LENGTH);
	int argSize = BUFF_LENGTH;
	int argPos = 0;

	if(!arg_v || !arg)
	{
		perror("Allocation error");
		exit(EXIT_FAILURE);		
	}

	for(int buffPos = 0; buff[buffPos] != '\0'; buffPos++)
	{
		if (isspace(buff[buffPos]))
		{
			// Skip multiple whitespaces
			if (argPos == 0)
			{
				continue;
			}
			arg[argPos] = '\0';
			arg_v[arg_vPos++] = arg;
			arg = malloc(BUFF_LENGTH);
			argPos = 0;
			if (!arg)
			{
				perror("Allocation error");
				exit(EXIT_FAILURE);
			}
		}
		else
		{
			arg[argPos++] = buff[buffPos];
		}

		if (argPos >= argSize)
		{
			argSize *= 2;
			arg = realloc(arg, argSize);
			if (!arg)
			{
				perror("Allocation error");
				exit(EXIT_FAILURE);
			}
		}

		if (arg_vPos >= arg_vSize)
		{
			arg_vSize *= 2;
			arg_v = realloc(arg_v, sizeof(char*) * arg_vSize);
			if (!arg_v)
			{
				perror("Allocation error");
				exit(EXIT_FAILURE);
			}
		}
	}

	// If currently 0 non-whitespace symbols are read, don't add new empty argument to arg_v
	if (argPos != 0)
	{
		arg[argPos] = '\0';
		arg_v[arg_vPos++] = arg;
		arg_v[arg_vPos] = NULL;
	}
	else
	{
		arg_v[arg_vPos] = NULL;
		free(arg);
	}
	return arg_v;
}

/**
 * Execute binary, system command or built-in command.
 *  
 * @param 	char **arg_v 	command and arguments
 * @return 	0 on success, -1 otherwise
 */
int shExecute(char **arg_v) 
{
	int status = 0;

	if (!arg_v)
	{
		return 0;
	}

	for(int i = 0; i < builtinLen(); i++)
	{
		if (strcmp(builtinNames[i], arg_v[0]) == 0)
		{
			status = (*builtinFunc[i])(arg_v);
			return status;
		}
	}

	int pid = fork();
	if (pid == 0)
	{
		if (execvp(arg_v[0], arg_v) == -1) 
		{
			perror("Can't create process");
			exit(EXIT_FAILURE);
		}
	}
	else 
	{
		wait(&status);
		return status;
	}
}
