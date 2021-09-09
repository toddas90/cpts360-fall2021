#include <stdio.h>
#include <stdlib.h>

#include "../include/quit.h"
#include "../include/tree.h"
//Improve to SAVE current tree to be reloadable

extern Node *root;

int quit() {
    printf("Program exit\n");
    deallocate(root);
    exit(0);
}
