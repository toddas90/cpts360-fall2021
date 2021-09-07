#include <stdio.h>

#include "tree.h"
#include "ls.h"

extern Node *root, *cwd, *start;

// Must improve to take in a pathname instead of
// just using the CWD.
void ls(char *pathname) {
    Node *p = cwd->child;
    printf("CWD Contents -> ");
    while (p) {
        printf("[%c %s] ", p->type, p->name);
        p = p->sibling;
    }
    printf("\n");
}
