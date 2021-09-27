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

int shell_launch_pipe(char **head, char **tail) {
    int pid1, pid2, pd[2], status;

    if (pipe(pd) < 0)
        perror("Pipe");

    pid1 = fork();

    if (pid1 == 0) {
        if (debug == 1)
            printf("child sh %d running %s\n", getpid(), head[0]);
        close(pd[0]); // Close read end
        dup2(pd[1], STDOUT_FILENO);
        close(pd[1]);
        if (execvp(head[0], head) == -1)
            perror("Shell");
    } else if (pid1 < 0)
        perror("Shell");
    else {
        if (debug == 1) {
            printf("sh %d forked a child sh %d\n", getpid(), pid1);
            printf("sh %d wait for child sh %d to terminate\n", getpid(), pid1);
        }

        pid2 = fork();

        if (pid2 == 0) {
            if (debug == 1)
                printf("child sh %d running %s\n", getpid(), tail[0]);
            close(pd[1]); // Close write end
            dup2(pd[0], STDIN_FILENO);
            close(pd[0]);
            shell_execute(tail, 1);
        } else if (pid2 < 0)
            perror("Shell");
        else {
            if (debug == 1) {
                printf("sh %d forked a child sh %d\n", getpid(), pid2);
                printf("sh %d wait for child sh %d to terminate\n", getpid(), pid2);
            }
            pid1 = wait(&status);
            close(pd[1]); // need to close the pipe so the other program knows to exit.
            if (debug == 1)
                printf("ZOMBIE child=%d exitStatus=%x\n", pid1, status);
            pid2 = wait(&status);
            close(pd[0]); // Just to make sure this side is also closed. Probably not needed.
            if (debug == 1)
                printf("ZOMBIE child=%d exitStatus=%x\n", pid2, status);
        }
    }
    return 1;
}

// Launches programs
int shell_launch(char **args, int ispipe) {
    int pid, status;

    if (ispipe == 1) {
        if (execvp(args[0], args))
            perror("Shell");
        return 1;
    }

    pid = fork();

    // Just some error checking
    if (pid == 0) {
        if (debug == 1)
            printf("child sh %d running %s\n", getpid(), args[0]);
        if (execvp(args[0], args) == -1)
            perror("Shell");
    } else if (pid < 0)
        perror("Shell");
    else {
        if (debug == 1) {
            printf("sh %d forked a child sh %d\n", getpid(), pid);
            printf("sh %d wait for child sh %d to terminate\n", getpid(), pid);
        }
        pid = wait(&status);
        if (debug == 1)
            printf("ZOMBIE child=%d exitStatus=%x\n", pid, status);
    }
    return 1; 
}

// Execute built in commands or launches program
int shell_execute(char **args, int ispipe) {
    int i = 0, j = 0, k, status = 0;

    if (args[0] == NULL)
        return 1;

    if (debug == 1) {
        for(k = 0; args[k] != NULL; k++)
            printf("args[%d]: %s\n", k, args[k]);
    }

    // Checks to see if the command is one of the built-in ones
    for (i = 0; i < shell_numcommands(); i++) {
        if (strcmp(args[0], commands_str[i]) == 0)
            return (*commands_fn[i])(args);
    }

    // Checks for i/o redirection
    while (args[j] != NULL) {
        if (!strcmp(args[j], "|")) {
            if (debug == 1)
                printf("Pipe detected!\n");
            args[j] = NULL;
            return shell_launch_pipe(&args[0], &args[j+1]);
        }

        if (!strcmp(args[j], ">")) {
            if (debug == 1)
                printf("Redirect: Create/Overwrite file!\n");
            args[j] = NULL;
            if (freopen(args[j+1], "w+", stdout) == NULL) // Create/Overwrite file
                perror("Shell");
            status = shell_launch(args, ispipe);
        } else if (!strcmp(args[j], ">>")) {
            if (debug == 1)
                printf("Redirect: Create/Append to file!\n");
            args[j] = NULL;
            if (freopen(args[j+1], "a+", stdout) == NULL) // Create/Append file
                perror("Shell");
            status = shell_launch(args, ispipe);
        } else if (!strcmp(args[j], "<")) {
            if (debug == 1)
                printf("Redirect: Read from file!\n");
            args[j] = NULL;
            if (freopen(args[j+1], "r", stdin) == NULL) // Read from file
                perror("Shell");
            status = shell_launch(args, ispipe) + 1;
        } else {
            j++;
        }
    }

    // If output has been redirected, change it back
    if (status == 1) {
        if (debug == 1)
            printf("Redirect: Terminal output!\n");
        if (freopen("/dev/tty", "w", stdout) == NULL)
            perror("Shell");
        return 1;
    } else if (status == 2) {
        if (debug == 1)
            printf("Redirect: Keyboard input!\n");
        if (freopen("/dev/tty", "r", stdin) == NULL)
            perror("Shell");
        return 1;
    }

    // Run non-built-in programs
    return shell_launch(args, ispipe);
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

    if (debug == 1)
        printf("Line: %s\n", line);

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
        if (debug == 1)
            printf("Current Token: %s\n", token);
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
        status = shell_execute(args, 0);

        free(args);
        free(line);
    } while (status);
}
