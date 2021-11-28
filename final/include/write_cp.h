#ifndef WRITE_CP_H
#define WRITE_CP_H

#include "type.h"

int my_write(int fd, char *buf, int nbytes);
int cp(char *src, char *dest);

#endif /* WRITE_CP_H */