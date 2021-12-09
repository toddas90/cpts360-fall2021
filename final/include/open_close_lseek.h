#ifndef OPEN_CLOSE_LSEEK_H
#define OPEN_CLOSE_LSEEK_H

#include "type.h"

int my_open(char *filename, int flags);
int my_lseek(int fd, int pos);
int my_close(int fd); 

void print_fd();

#endif /* OPEN_CLOSE_LSEEK_H */