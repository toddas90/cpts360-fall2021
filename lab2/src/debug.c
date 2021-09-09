#include <stdio.h>

#include "../include/debug.h"
#include "../include/tree.h"

extern Node *root, *cwd, *start;

void print_ptr() {
    Node *p;
    printf("Root -> %p -> %s\n", root, root->name);
    printf("CWD -> %p -> %s\n", cwd, cwd->name);
    printf("CWD Parent -> %p -> %s\n", cwd->parent, cwd->parent->name);
    
    p = cwd->parent->child;
    if (cwd != root) {
        printf("CWD Siblings: ");
        while (p) {
            printf("%p -> %s, ", p, p->name);
            p = p->sibling;
        }
        printf("\n");
    }

    p = cwd->child;
    if (p == NULL)
        return;
    printf("CWD Children: ");
    while (p) {
        printf("%p -> %s, ", p, p->name);
        p = p->sibling;
    }
    printf("\n");
}
