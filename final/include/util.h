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

int midealloc(MINODE *mip);

int findino(MINODE *m, u32 *my);

#endif /* UTIL_H */
