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

// Launches programs
int shell_launch(char **args) {
    int pid, status;

    pid = fork();

    // Just some error checking
    if (pid == 0) {
        // Runs the program and throws error if it failed
        if (execvp(args[0], args) == -1) {
            perror("Shell: ");
        }
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        // pid is a negative number which is bad
        perror("Shell: ");
    } else {
        // Wait for the child
        pid = wait(&status);
    }
    return 1; 
}

// Execute built in commands or launches program
int shell_execute(char **args) {
    int i = 0, j = 0, status = 0;
    FILE *fp = NULL;

    if (args[0] == NULL) {
        // Args can't be empty
        return 1;
    }

    // Checks to see if the command is one of the built-in ones
    for (i = 0; i < shell_numcommands(); i++) {
        if (strcmp(args[0], commands_str[i]) == 0) {
            // If so, it will run it
            return (*commands_fn[i])(args);
        }
    }

    // Checks for i/o redirection
    while (args[j] != NULL) {
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
        if (feof(stdin)) {
            exit(EXIT_SUCCESS);
        } else  {
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
        if (getcwd(dir, PATH_MAX) == NULL) {
            perror("getcwd() error");
        }
        printf("%s@%s %s > ", username, host, dir);
        line = shell_readline();
        args = shell_parseline(line);
        status = shell_execute(args);

        free(line);
        free(args);
    } while (status);
}
