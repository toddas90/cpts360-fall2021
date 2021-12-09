#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <ext2fs/ext2_fs.h>
#include <string.h>
#include <libgen.h>
#include <sys/stat.h>
#include <time.h>

#include "../include/type.h"
#include "../include/util.h"
#include "../include/terminal.h"
#include "../include/switch.h"

extern MINODE minode[NMINODE];
extern MINODE *root;
extern PROC   proc[NPROC], *running;
extern MOUNT mountTable[NMOUNT];

extern int fd, dev, rootdev;
extern char line[128], cmd[32], pathname[128], extra_arg[128];

int my_switch(char *user) {
    if (strlen(user) == 0) {
        printf(YEL "Username not provided\n" RESET);
        return -1;
    }
    
    if (!strcmp("su", user)) {
        running = &proc[0];
    }
    if (!strcmp("P1", user)) {
        running = &proc[1];
    }

    return 0;
}