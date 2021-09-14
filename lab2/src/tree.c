#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../include/tree.h"
#include "../include/parse.h"

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
    // printf("Search for %s in parent DIR\n", name);
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
    // printf("Insert Node %s into END of parent child list\n", q->name);
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

Node *find_node(char *path, Node *begin) {
    Node *p;
    // printf("searching for %s starting in %s\n", path, begin->name);
    if (start->child == NULL) {
        return NULL;
    } else {
        p = begin->child;
        while(p) {
            if (strcmp(p->name, path) == 0)
                return p;
            p = p->sibling;
        }
        return NULL;
    }    
}

void delete(Node *p) {
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

void deallocate(Node *node) {
    if(node == 0)
        return;

    deallocate(node->sibling);
    deallocate(node->child);
    free(node);
}
