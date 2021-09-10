#include <stdio.h>
#include <string.h>

#include "../include/rm.h"
#include "../include/tree.h"

extern Node *cwd, *root, *start;

void rm(char *path) {
    Node *p;
    if (path[0] == '/') {
        p = find_node(path, root);
    } else {
        p = find_node(path, cwd);
    }
    if (p == NULL) {
        printf("File %s not found\n", path);
        return;
    }
    if (p->type == 'D') {
        printf("%s is a directory\n", path);
        return;
    }

    if (!strcmp(path, "/") || !strcmp(path, cwd->name)) {
        printf("Cannot remove %s\n", path);
        return;
    }
    delete(p);
}
