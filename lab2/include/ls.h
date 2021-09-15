#ifndef LS_H
#define LS_H

#include "tree.h"

void ls(char *path);

Node *ls_helper(char *path, char *pt);

#endif /* LS_H */
