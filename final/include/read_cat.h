#ifndef READ_CAT_H
#define READ_CAT_H

#include "type.h"

int my_read(int fd, char *buf, int nbytes);
int cat(char *file);

#endif /* READ_CAT_H */