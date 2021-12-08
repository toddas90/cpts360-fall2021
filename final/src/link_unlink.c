#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <ext2fs/ext2fs.h>
#include <sys/stat.h>
#include <libgen.h>

#include "../include/link_unlink.h"
#include "../include/rmdir.h"
#include "../include/mkdir_creat.h"
#include "../include/util.h"
#include "../include/terminal.h"
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

    int src_ino = getino(pathname);
    MINODE *src_mip = iget(dev, src_ino);

    if (S_ISDIR(src_mip->INODE.i_mode)) {
        printf(YEL "Cannot link a directory\n" RESET);
        return -1;
    }

    if (getino(extra_arg) != 0) {
        printf(YEL "File already exists\n" RESET);
        return -1;
    }

    char *path2 = strdup(extra_arg);
    char *parent = dirname(extra_arg);
    char *child = basename(path2);
    
    int dest_ino = getino(parent);
    MINODE *dest_mip = iget(dev, dest_ino);
    enter_child(dest_mip, src_ino, child);

    src_mip->INODE.i_links_count += 1;
    src_mip->dirty = 1;
    iput(src_mip);
    iput(dest_mip);
    return 0;
}

int my_unlink() {
    //printf("Unlinking %s\n", pathname);
    int ino = getino(pathname);
    MINODE *mip = iget(dev, ino);

    if (S_ISDIR(mip->INODE.i_mode)) {
        printf(YEL "File is a dir\n" RESET);
        return -1;
    }

    char *path2 = strdup(pathname);
    char *parent = dirname(pathname);
    char *child = basename(path2);

    int pino = getino(parent);
    MINODE *pmip = iget(dev, pino);
    rm_child(pmip, child);
    pmip->dirty = 1;
    iput(pmip);

    mip->INODE.i_links_count -= 1;
    if (mip->INODE.i_links_count > 0)
        mip->dirty = 1;
    else {
        // for (int i = 0; i < mip->INODE.i_size/BLKSIZE; i++) {
        //     if (mip->INODE.i_block[i] == 0)
        //         break;
        //     bdalloc(dev, pmip->INODE.i_block[i]);
        // }
        bdalloc(dev, ino);
        idalloc(dev, ino);
    }
    iput(mip);
    return 0;
}
