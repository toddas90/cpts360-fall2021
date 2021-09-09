#include <stdio.h>
#include <string.h>

#include "../include/tree.h"
#include "../include/rmdir.h"

extern Node *cwd, *root, *start;

void rmdir(char *path) {
    Node *p;
    p = find_node(path);

    // CASE -1
    if (p == NULL) {
        printf("Directory %s not found\n", path);
        return;
    }
    
    // CASE 0
    if (p->child != NULL) {
        printf("Directry not empty!\n");
        return;
    }

    // CASE 1
    if (!strcmp(path, "/") || !strcmp(path, cwd->name)) {
        printf("Cannot remove %s\n", path);
        return;
    }

    // CASE 2
    if (p->parent->child == p) {
        if (p->sibling == NULL) {
            p->parent->child = NULL;
        } else {
            p->parent->child = p->sibling;
            p->sibling = NULL;
        }
        deallocate(p);
        return;
    }

    if (p->parent->child != p) {
        Node *j = p->parent->child;
        while (j->sibling != p)
            j = j->sibling;
        if (p->sibling != NULL) {
            j->sibling = p->sibling;
            p->sibling = NULL;
        } else {
            j->sibling = NULL;
        }
        deallocate(p);
        return;
    }
}
