#ifndef COMMAND_H
#define COMMAND_H

#define HOME getenv("HOME")

typedef struct
{
    char** arguments; // array to store arguments
    int argumentsCount; // stores the number of arguments
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
} Command;

Command* CommandInit();
void CommandDel(Command *cmd);

void CommandAddRedirection(Command *cmd, int channel, char *fname, int flags);
void CommandOpenFiles(Command *cmd);
void CommandHandlePipe(Command *cmd);

int CommandExecute(Command *cmd);
int CommandPartialExecute(Command *cmd, int first, int last);

void CommandResetRedirections(Command *cmd);
int CommandEraseArguments(Command *cmd, int first, int last);

// Builtin Command: Exit
int CommandExit(Command *cmd);
// Builtin Command: Chdir
int CommandChdir(Command *cmd);

#endif //COMMAND_H