#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../include/rmdir.h"
#include "../include/tree.h"
#include "../include/parse.h"
#include "../include/cd.h"

extern Node *cwd, *root, *start;

void rmdir(char *path) {
    Node *p;

    if (!strcmp(path, "")) {
        printf("No name provided\n");
        return;
    }
    
    char *name = malloc(strlen(path) + 1);
    strcpy(name, path);

    char *tok_last = malloc(strlen(path) + 1);

    char *token = parse(name);
    start = cwd;

    while (token) {
        strcpy(tok_last, token);
        token = strtok(NULL, "/");
    }

    if (find_node(tok_last, cwd) == NULL) {
        cd(name);
    }
    // cd(name);

    p = find_node(tok_last, cwd);

    if (p == NULL) {
        printf("Directory %s not found\n", tok_last);
        free(name);
        free(tok_last);
        cd(start->name);
        return;
    }
    if (p->type == 'F') {
        printf("%s is a file\n", tok_last);
        free(name);
        free(tok_last);
        cd(start->name);
        return;
    }

    if (p == root || !strcmp(tok_last, cwd->name)) {
        printf("Cannot remove %s\n", tok_last);
        free(name);
        free(tok_last);
        cd(start->name);
        return;
    }

    if (p->child != NULL) {
        printf("Directory is not empty\n");
        free(name);
        free(tok_last);
        cd(start->name);
        return;
    }

    delete(p);
    free(name);
    free(tok_last);
    cd(start->name);
}
