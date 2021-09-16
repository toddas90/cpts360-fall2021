#include <stdio.h>
#include <string.h>
#include <libgen.h>

#include "../include/tree.h"
#include "../include/cd.h"
#include "../include/parse.h"

extern Node *root, *cwd, *start;

// Can't cd into a sibling directory.

void cd(char *path) {
    if (path[0] == '/') {
        //printf("Start from /\n");
        start = root;
    }
    else {
        //printf("Start from cwd\n");
        start = cwd;
    }

    //printf("CD Path: %s\n", path);
    char *token = parse(path);
    if (token == NULL) {
        //printf("cd into /\n");
        cwd = root;
        return;
    }

    while (token) {
        if (!strcmp(token, "/")) {
            //printf("cd into /\n");
            cwd = root;
            //return;
        }
        if (!strcmp(token, "..")) {
            //printf("cd into parent\n");
            cwd = cwd->parent;
            //return;
        }
        // printf("CD: Current Token: %s\n", token);
        Node *p = find_node(token, cwd);
        if (p != NULL) {
            if (p->type == 'F') {
                //printf("CD: %s is not a directory\n", token);
                return;
            }
            cwd = p;
        }
        //else
            // printf("CD: Can't CD into %s\n", path);
    token = strtok(NULL, "/");
    }
}

