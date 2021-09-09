#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../include/tree.h"

extern Node *root, *start, *cwd;

void initialize() {
    root = (Node *)malloc(sizeof(Node));
    strcpy(root->name, "/");
    root->parent = root;
    root->sibling = NULL;
    root->child = NULL;
    root->type = 'D';
    cwd = root;
    printf("Root init OK\n");
}

Node *search_child(Node *parent, char *name) {
    Node *p;
    printf("Search for %s in parent DIR\n", name);
    p = parent->child;
    if (p == NULL)
        return NULL;
    while (p) {
        if (strcmp(p->name, name) == 0)
            return p;
        p = p->sibling;
    }
    return NULL;
}

void insert_child(Node *parent, Node *q) {
    Node *p;
    printf("Insert Node %s into END of parent child list\n", q->name);
    p = parent->child;
    if (p == NULL)
        parent->child = q;
    else {
        while (p->sibling)
            p = p->sibling;
        p->sibling = q;
    }
    q->parent = parent;
    q->child = NULL;
    q->sibling = NULL;
}

void deallocate(Node *node) {
    if(node == 0)
        return;

    deallocate(node->sibling);
    deallocate(node->child);
    free(node);
}
