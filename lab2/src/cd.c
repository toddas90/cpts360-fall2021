#include <stdio.h>
#include <string.h>

#include "../include/tree.h"
#include "../include/cd.h"

extern Node *root, *cwd, *start;

// Train of thought:
// The node creation is messing up the name and/or parent assignment
// inside of either mkdir.c or tree.c

void cd(char *path) {
    int b = cd_helper(path);
    if (b == 2) {
        Node *p = search_child(root, path);
        if (p != NULL)
            cwd = p;
        else
            printf("Can't CD into %s", path);
    }
    else if (b == 0)
        cwd = root; 
    else if (b == 1)
        cwd = cwd->parent;
}

int cd_helper(char *path) {
    if (!strcmp(path, "")) {
        printf("cd into /\n");
        return 0;
    } else if (!strcmp(path, "..")) {
        printf("cd into parent\n");
        return 1;
    } else {
        printf("cd into %s\n", path);
        return 2;
    }
}
