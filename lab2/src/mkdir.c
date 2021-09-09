#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../include/tree.h"
#include "../include/mkdir.h"

extern Node *start, *cwd, *root;
//Currently takes name, modify to take pathname
//(currently only mkdir in CWD)
int mkdir(char *name) {
    Node *p, *q;
    printf("Mkdir: name = %s\n", name);

    if (!strcmp(name, "/") || !strcmp(name, ".") ||
            !strcmp(name, "..")) {
        printf("Can't mkdir with %s\n", name);
        return -1;
    }
    if (name[0] == '/')
        start = root;
    else
        start = cwd;

    printf("Check wether %s already exists\n", name);
    p = search_child(start, name);
    if (p) {
        printf("name %s already exists, mkdir failed\n", name);
        return -1;
    }
    q = (Node *)malloc(sizeof(Node));
    q->type = 'D';
    strcpy(q->name, name);
    insert_child(start, q);
    printf("Mkdir %s OK\n", name);
    return 0;
}
