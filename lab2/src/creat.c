#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../include/tree.h"
#include "../include/creat.h"

extern Node *start, *cwd, *root;
//Currently takes name, modify to take pathname
//(currently only creat in CWD)
int creat(char *name) {
    Node *p, *q;
    printf("Creat: name = %s\n", name);

    if (!strcmp(name, "/") || !strcmp(name, ".") ||
            !strcmp(name, "..")) {
        printf("Can't creat with %s\n", name);
        return -1;
    }
    if (name[0] == '/')
        start = root;
    else
        start = cwd;

    printf("Check wether %s already exists\n", name);
    p = search_child(start, name);
    if (p) {
        printf("name %s already exists, creat failed\n", name);
        return -1;
    }
    q = (Node *)malloc(sizeof(Node));
    q->type = 'F';
    strcpy(q->name, name);
    insert_child(start, q);
    printf("Creat %s OK\n", name);
    return 0;
}
