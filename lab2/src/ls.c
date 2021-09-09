#include <stdio.h>
#include <string.h>

#include "../include/tree.h"
#include "../include/ls.h"

extern Node *root, *cwd, *start;

// Must improve to take in a pathname instead of
// just using the CWD.
void ls(char *pathname) {
    Node *p = ls_helper(pathname);
    printf("Contents -> ");
    while (p) {
        printf("[%c %s] ", p->type, p->name);
        p = p->sibling;
    }
    printf("\n");
}

Node *ls_helper(char *path) {
    if (strcmp(path, " ")) {
        //return cwd->child;
        return cwd->child;
    } else {
        return search_child(root, path);
    }
}
