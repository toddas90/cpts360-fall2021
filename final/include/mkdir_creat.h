#ifndef MKDIR_CREAT_H
#define MKDIR_CREAT_H

#include "../include/type.h"

int lab_mkdir();
int lab_creat();

int ialloc(int dev);
int balloc(int dev);
int enter_child(MINODE *pmip, int ino, char *basename);

int test_bit(char *buf, int bit);
int set_bit(char *buf, int bit);
int dec_free_inodes(int dev);

#endif /* MKDIR_CREAT_H */
