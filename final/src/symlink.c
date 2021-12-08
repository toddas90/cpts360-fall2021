#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <ext2fs/ext2fs.h>
#include <sys/stat.h>
#include <libgen.h>

#include "../include/symlink.h"
#include "../include/link_unlink.h"
#include "../include/rmdir.h"
#include "../include/mkdir_creat.h"
#include "../include/util.h"
#include "../include/terminal.h"
#include "../include/type.h"

extern char pathname[128], extra_arg[128];
extern int dev;
extern PROC *running;

int my_symlink() {
    if (!strcmp(pathname, "")) {
        printf(YEL "Source file not found\n" RESET);
        return -1;
    }

    if (!strcmp(extra_arg, "")) {
        printf(YEL "New file not provided\n" RESET);
        return -1;
    }

    int src_ino = getino(pathname);
    if (src_ino <= 0) {
        printf(YEL "File does not exist\n" RESET);
        return -1;
    }

    MINODE *src_mip = iget(dev, src_ino);

    if (getino(extra_arg) != 0) {
        printf(YEL "File already exists\n" RESET);
        return -1;
    }

    src_mip->INODE.i_links_count += 1;
    //src_mip->dirty = True;
    iput(src_mip);

    char buf[BLKSIZE];

    my_creat(extra_arg); // create new file

    int dest_ino = getino(extra_arg);

    MINODE *dest_mip = iget(dev, dest_ino);

    dest_mip->INODE.i_mode = 0xA1FF; // set link type
   
    DIR *dp = (DIR *)buf;
    char *cp = buf;

    get_block(dev, dest_mip->INODE.i_block[0], buf);
    strncpy(dp->name, pathname, strlen(pathname));
    put_block(dev, dest_mip->INODE.i_block[0], buf);
    dest_mip->INODE.i_size = strlen(pathname);

    dest_mip->dirty = True;
    iput(dest_mip);
    return 0;
}
