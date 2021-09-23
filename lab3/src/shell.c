#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <limits.h>

#include <sys/wait.h>

#include "../include/shell.h"
#include "../include/commands.h"

#define TOK_BUFSIZE 64
#define TOK_DELIM " \t\r\n\a"

int shell_pipe(char **head, char **tail) {
    int pid, pd[2];

    if (pipe(pd) < 0)
        exit(EXIT_FAILURE);

    pid = fork();

    if (pid == 0) {
        close(pd[0]);
        close(1);
        dup(pd[1]);
        close(pd[1]);
        if (execvp(head[0], head) == -1)
            perror("Shell");
    } else if (pid < 0)
        perror("Shell");
    else {
        close(pd[1]);
        close(0);
        dup(pd[0]);
        close(pd[0]);
        if (execvp(tail[0], tail) == -1)
            perror("Shell");
    }
    return 1;
}

// Launches programs
int shell_launch(char **args) {
    int pid, status;

    pid = fork();

    // Just some error checking
    if (pid == 0) {
        if (execvp(args[0], args) == -1)
            perror("Shell");
        exit(EXIT_FAILURE);
    } else if (pid < 0)
        perror("Shell");
    else
        pid = wait(&status);
    return 1; 
}

// Execute built in commands or launches program
int shell_execute(char **args) {
    int i = 0, j = 0, status = 0;
    FILE *fp = NULL;

    if (args[0] == NULL)
        return 1;

    // Checks to see if the command is one of the built-in ones
    for (i = 0; i < shell_numcommands(); i++) {
        if (strcmp(args[0], commands_str[i]) == 0)
            return (*commands_fn[i])(args);
    }

    // Checks for i/o redirection
    while (args[j] != NULL) {
        if (!strcmp(args[j], "|")) {
            args[j] = NULL;
            return shell_pipe(&args[0], &args[j+1]);
        }

        if (!strcmp(args[j], ">")) {
            args[j] = NULL;
            fp = freopen(args[j+1], "w+", stdout); // Create/Overwrite file
            if (fp)
                status = shell_launch(args);
        } else if (!strcmp(args[j], ">>")) {
            args[j] = NULL;
            fp = freopen(args[j+1], "a+", stdout); // Create/Append file
            if (fp)
                status = shell_launch(args);
        } else if (!strcmp(args[j], "<")) {
            args[j] = NULL;
            fp = freopen(args[j+1], "r", stdin); // Read from file
            if (fp)
                status = shell_launch(args) + 1;
        } else {
            j++;
        }
    }

    // If output has been redirected, change it back
    if (status == 1) {
        fp = freopen("/dev/tty", "w", stdout);
        if (fp)
            return 1;
        return 0;
    } else if (status == 2) {
        fp = freopen("/dev/tty", "r", stdin);
        if (fp)
            return 1;
        return 0;
    }

    // Run non-built-in programs
    return shell_launch(args);
}

// Read line into program
char *shell_readline() {
    char *line = NULL;
    size_t bufsize = 0;

    // Reads a line into buffer
    if (getline(&line, &bufsize, stdin) == -1){
        if (feof(stdin))
            exit(EXIT_SUCCESS);
        else  {
            perror("Readline: ");
            exit(EXIT_FAILURE);
        }
    }
    return line;
}

// Parse line (on whitespace)
char **shell_parseline(char *line) {
    int bufsize = TOK_BUFSIZE, pos = 0;
    char **tokens = malloc(bufsize * sizeof(char*));
    char *token;

    // Delimiter for tokens is set up top
    token = strtok(line, TOK_DELIM);
    while (token != NULL) {
        tokens[pos] = token;
        pos++;
        token = strtok(NULL, TOK_DELIM);
    }
    tokens[pos] = NULL;
    return tokens;
}

// Main shell loop
void shell_loop(void) {
    char *line, *username;
    char host[HOST_NAME_MAX+1], dir[PATH_MAX];
    char **args;
    int status = 0;

    gethostname(host, HOST_NAME_MAX+1);
    username = getenv("USER");

    do {
        if (getcwd(dir, PATH_MAX) == NULL)
            perror("getcwd() error");

        printf("%s@%s %s > ", username, host, dir);
        
        line = shell_readline();
        args = shell_parseline(line);
        status = shell_execute(args);

        free(args);
        free(line);
        printf("Status: %d\n", status);
    } while (status);
}
