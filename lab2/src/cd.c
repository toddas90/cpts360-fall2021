#include <stdio.h>
#include <string.h>
#include <libgen.h>

#include "../include/tree.h"
#include "../include/cd.h"
#include "../include/parse.h"

extern Node *root, *cwd, *start;

void cd(char *path) {
    if (!strcmp(path, "/")) {
        printf("cd into /\n");
        cwd = root;
        return;
    }
    char *token = parse(path);
    if (token == NULL) {
        printf("cd into /\n");
        cwd = root;
        return;
    }
    while (token) {
        int b = cd_helper(token);
        if (b == 2) {
            Node *p;
            if (path[0] == '/')
                p = find_node(token, root);
            else
                p = find_node(token, cwd);
            if (p != NULL) {
                if (p->type == 'F') {
                    printf("%s is not a directory", path);
                    return;
                }
                cwd = p;
            }
            else
                printf("Can't CD into %s\n", path);
        }
        else if (b == 1) {
            cwd = cwd->parent;
        }
        token = strtok(NULL, "/");
        }
}

int cd_helper(char *path) {
    if (!strcmp(path, "..")) {
        printf("cd into parent\n");
        return 1;
    } else {
        printf("cd into %s\n", path);
        return 2;
    }
}
