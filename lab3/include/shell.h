#ifndef SHELL_H
#define SHELL_H

void shell_loop(void);

char **shell_parseline(char *line);

char *shell_readline(void);

int shell_execute(char **args, int ispipe);

int shell_launch(char **args, int ispipe);

#endif /* SHELL_H */
