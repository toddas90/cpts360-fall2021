#include <stdio.h>

#include "../include/pwd.h"
#include "../include/tree.h"

extern Node *root, *cwd, *start;

void pwd(Node *node) {
    if (node == root) {
        printf("%p -> %s\n", &node->parent, node->name);
        printf("%s", node->name);
        return;
    }
    printf("%p -> %s\n", &node->parent, node->name);
    pwd(node->parent); 
    printf("%s/", node->name);
}
