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
#include "../include/colors.h"
#include "../include/type.h"

extern char pathname[128], extra_arg[128];
extern int dev;
extern PROC *running;

int symlink() {
    if (!strcmp(pathname, "")) {
        printf(YEL "Source file not found\n" RESET);
        return -1;
    }

    if (!strcmp(extra_arg, "")) {
        printf(YEL "New file not provided\n" RESET);
        return -1;
    }

    int old_ino = getino(pathname);
    if (old_ino <= 0) {
        printf(YEL "File does not exist\n" RESET);
        return -1;
    }

    MINODE *old_mip = iget(dev, old_ino);

    if (getino(extra_arg) != 0) {
        printf(YEL "File already exists\n" RESET);
        return -1;
    }

    char *copy_pathname[128];
    strcpy(copy_pathname, pathname);
    strcpy(pathname, extra_arg);

    lab_creat(); // create new file

    int ino = getino(extra_arg); // get new file
    MINODE *mip = iget(dev, ino);

    mip->INODE.i_mode = 0xA1FF; // set link type

    // NEED TO FINISH
    
    // assume length of old_file name <= 60 chars
        // store old_file name in newfile's INODE.i_block[] area
        // set file size to length of old_file name
        // mark new_file's minode dirty
        // iput new_file
    // mark new_file's parent minode dirty
    // iput new_file's parent.

    return 0;
}