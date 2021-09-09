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
    if (p->parent->child == p && p->sibling == NULL) {
        printf("only child removal\n");
        p->parent->child = NULL;
        deallocate(p);
        return;
    }

    // CASE 3
    if (p->parent->child == p && p->sibling != NULL) {
        printf("first child with sibling removal\n");
        p->parent->child = p->sibling;
        p->sibling = NULL;
        deallocate(p);
        return;
    }

    // CASE 4
    if (p->parent->child != p && p->sibling != NULL) {
        printf("Sibling with siblings removal\n");
        Node *j = p->parent->child;
        while (j->sibling != p)
            j = p->sibling;
        j->sibling = p->sibling;
        p->sibling = NULL;
        deallocate(p);
        return;
    }

    // CASE 5 (broken)
    if (p->parent->child != p && p->sibling == NULL) {
        printf("Sibling with no siblings removal\n");
        Node *j = p->parent->child;
        while (j->sibling != p)
            j = p->sibling;
        j->sibling = NULL;
        deallocate(p);
        return;
    }
}

// RMDIR CASES:
// -1: DIR not found
// 0: DIR is not empty
// 1: DIR is cwd or root
// 2: DIR is child of parent with NO siblings
// 3: DIR is child of parent with siblings
// 4: DIR is sibling with siblings
// 5: DIR is sibling with NO siblings
