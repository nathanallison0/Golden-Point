#include <stdlib.h>

typedef struct {
    char* prefix;
    void (*handler)(int, char **);
} DT_Command;

typedef struct {
    const DT_Command* commands;
    size_t num_commands;
} DT_Interpreter;

DT_Interpreter *DT_CreateInterpreter(const DT_Command *commands, size_t num_commands);

void DT_DestroyInterpreter(DT_Interpreter *interpreter);

const char *DT_GetConsole(void);

void DT_ClearConsole(void);

void DT_ConsolePrint(char *text);

void DT_ConsolePrintln(char *text);

void DT_ConsolePrintf(const char *format, ...);

int DT_InterpretCommand(DT_Interpreter *interpreter, char *text);

void DT_CombineArgs(char **cmd_args, int start_arg, int num_args, char sep);