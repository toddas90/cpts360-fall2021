#include <stdio.h>
#include <string.h>

#include "../include/tree.h"
#include "../include/ls.h"

extern Node *root, *cwd, *start;

void ls(char *pathname) {
    Node *p = ls_helper(pathname);
    if (p == NULL)
        return;
    if (p->type == 'F') {
        printf("%s is not a directory\n", pathname);
        return;
    }
    p = p->child;
    while (p) {
        printf("[%c %s] ", p->type, p->name);
        p = p->sibling;
    }
    printf("\n");
}

Node *ls_helper(char *path) {
    if (!strcmp(path, "")) {
        return cwd;
    } else {
        if (path[0] == '/')
            return find_node(path, root);
        else
            return find_node(path, cwd);
    }
}
