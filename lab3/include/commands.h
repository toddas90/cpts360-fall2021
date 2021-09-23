#ifndef COMMANDS_H
#define COMMANDS_H

extern int debug;

extern char *commands_str[];

extern int (*commands_fn[]) (char **);

int shell_debug(char **args);

int shell_exit(char **args);

int shell_cd(char **args);

int shell_numcommands(void);

#endif /* COMMANDS_H */
