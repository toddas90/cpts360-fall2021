#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <ext2fs/ext2fs.h>
#include <sys/stat.h>
#include <libgen.h>

#include "../include/link_unlink.h"
#include "../include/mkdir_creat.h"
#include "../include/util.h"
#include "../include/colors.h"
#include "../include/type.h"

extern char pathname[128], extra_arg[128];
extern int dev;
extern PROC *running;

int my_link() {
    if (!strcmp(pathname, "")) {
        printf(YEL "Source file not found\n" RESET);
        return -1;
    }

    if (!strcmp(extra_arg, "")) {
        printf(YEL "New file not provided\n" RESET);
        return -1;
    }

    int old_ino = getino(pathname);
    MINODE *old_mip = iget(dev, old_ino);

    if (S_ISDIR(old_mip->INODE.i_mode)) {
        printf(YEL "Cannot link a directory\n" RESET);
        return -1;
    }

    if (getino(extra_arg) != 0) {
        printf(YEL "File already exists\n" RESET);
        return -1;
    }

    char *parent = dirname(extra_arg);
    char *child = basename(extra_arg);
    int pino = getino(parent);
    MINODE *pmip = iget(dev, pino);
    enter_child(pmip, old_ino, child);

    old_mip->INODE.i_links_count += 1;
    old_mip->dirty = 1;
    iput(old_mip);
    iput(pmip);
    return 0;
}
