#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../include/tree.h"
#include "../include/ls.h"
#include "../include/parse.h"
#include "../include/cd.h"

extern Node *root, *cwd, *start;

void ls(char *pathname) {
    char *path = malloc(strlen(pathname) + 1);
    char *tok_last;

    strcpy(path, pathname);

    char *token = parse(pathname);

    while (token) {
        tok_last = token;
        //strcpy(tok_last, token);
        token = strtok(NULL, "/");
    }

    start = cwd;

    Node *p = ls_helper(path, tok_last);
    
    if (p == NULL) {
        free(path);
        cd(start->name);
        return;
    }
    if (p->type == 'F') {
        printf("%s is not a directory\n", pathname);
        free(path);
        cd(start->name);
        return;
    }
    p = p->child;
    while (p) {
        printf("[%c %s] ", p->type, p->name);
        p = p->sibling;
    }
    printf("\n");
    free(path);
    cd(start->name);
}

Node *ls_helper(char *path, char *srch) {
    if (!strcmp(path, "")) {
        return cwd;
    } else {
        cd(path);
        return cwd;
        //return find_node(srch, cwd);
    }
}
