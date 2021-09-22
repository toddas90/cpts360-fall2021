#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "../include/commands.h"

// Array of commands that are built into the shell
// Easily expandable
char *commands_str[] = {
    "cd",
    "exit"
};

// Array of function pointers to built-in commands
// Also makes it easily expandable in the future
int (*commands_fn[]) (char **) = {
    &shell_cd,
    &shell_exit
};


// Gets number of shell commands
int shell_numcommands(void) {
    return sizeof(commands_str) / sizeof(char *);
}

// Built-in cd command
int shell_cd(char **args) {
    if (args[1] == NULL) {
        fprintf(stderr, "Expected argument to \"cd\"\n");
    } else {
        if (chdir(args[1]) != 0) {
            perror("shell");
        }
    }
    return 1;
}

// Built-in exit command
int shell_exit(char **args) {
    return 0;
}
