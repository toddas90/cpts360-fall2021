#ifndef UTIL_H
#define UTIL_H

#include <ext2fs/ext2fs.h>
#include "type.h"

int get_block(int d, int b, char *buf);
int put_block(int d, int b, char *buf);
int tokenize(char *p);
MINODE *iget(int d, int i);
void iput(MINODE *m);
int search(MINODE *m, char *n);
int getino(char *p);
int findmyname(MINODE *p, u32 m, char *my);
MINODE *mialloc();
int midealloc(MINODE *mip);
int findino(MINODE *m, u32 *my);
void printblk(MINODE *mip);
int ideal_length(char *name);
int test_bit(char *buf, int bit);
int set_bit(char *buf, int bit);
int dec_free_inodes(int dev);
int ialloc(int dev);
int balloc(int dev);
int numblks(MINODE *mip);
int clr_bit(char *buf, int bit);
int inc_free_inodes(int dev);
int map(INODE ip, int lbk);
MOUNT *getmptr(int dev);

int idalloc(int dev, int ino);
int bdalloc(int dev, int bno);

#endif /* UTIL_H */
