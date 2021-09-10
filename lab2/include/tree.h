#ifndef TREE_H
#define TREE_H

typedef struct Node {
    char name[64];
    char type;
    struct Node *child, *sibling, *parent;
}Node;

Node *search_child(Node *parent, char *name);

void insert_child(Node *parent, Node *q);

void initialize();

Node *find_node(char *path, Node *begin);

void delete(Node *p);

void deallocate(Node *node);

#endif /* TREE_H */
