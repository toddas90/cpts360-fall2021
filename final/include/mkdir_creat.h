#ifndef MKDIR_CREAT_H
#define MKDIR_CREAT_H

#include "type.h"

int my_mkdir(char *pathname);
int my_creat(char *pathname);
int enter_child(MINODE *pmip, int ino, char *basename);

#endif /* MKDIR_CREAT_H */
