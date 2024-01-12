#pragma once

typedef enum SH_COMMAND
{
    C_HELP = 0,
    C_LS = 1,
} SH_COMMAND;

typedef struct
{
} Shell;

void InitShell();
void ShellHandleCommands();
void sh_newline();