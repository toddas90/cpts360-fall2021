#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../include/tree.h"
#include "../include/mkdir.h"
#include "../include/parse.h"
#include "../include/cd.h"

extern Node *start, *cwd, *root;

// If you are in / and try to make a dir /a/b, it will place b in / instead.

int mkdir(char *name) {
    Node *p, *q; 

    if (!strcmp(name, "")) {
        printf("No name provided\n");
        return -1;
    }

    char *path = malloc(strlen(name) + 1);
    strcpy(path, name);

    if (name[0] == '/')
        start = root;
    else
        start = cwd;

    cd(path);

    char *token = parse(name);
    char *tok_last;

    //printf("MKDIR: Path %s\n", name);
    while (token) {
        // printf("MKDIR: Token %s\n", token);
        tok_last = token;
        token = strtok(NULL, "/");
    }

    if (!strcmp(name, "/") || !strcmp(name, ".") ||
            !strcmp(name, "..")) {
        printf("Can't mkdir with %s\n", tok_last);
        cd(start->name);
        free(path);
        return -1;
    }

    p = search_child(start, tok_last);
    if (p) {
        printf("name %s already exists, mkdir failed\n", name);
        cd(start->name);
        free(path);
        return -1;
    }
   
    q = (Node *)malloc(sizeof(Node));
    q->type = 'D';
    strcpy(q->name, tok_last);
    insert_child(cwd, q);
    cd(start->name);
    free(path);
    return 0;
}
