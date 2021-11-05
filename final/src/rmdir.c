#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <ext2fs/ext2fs.h>
#include <sys/stat.h>

#include "../include/rmdir.h"
#include "../include/util.h"
#include "../include/colors.h"
#include "../include/type.h"

extern char pathname[128];
extern int dev;
extern PROC *running;

int rmdir() {
    if (!strcmp(pathname, "")) {
        printf("No path was provided\n");
        return -1;
    }

    int ino = getino(pathname);
    MINODE *mip = iget(dev, ino);
    
    if (!S_ISDIR(mip->INODE.i_mode)) {
        printf("Not a directory\n");
        return -1;
    }

    if (mip->refCount != 1) {
        printf("Directory busy\n");
        return -1;
    }

    if (numblks(mip) != 2) {
        printf("Dir not empty\n");
        return -1;
    }

    int pino = findino(mip, ino);
    MINODE *pmip = iget(mip->dev, pino);
    char name[64];
    findmyname(pmip, ino, name);
    //rm_child(pmip, name);

    pmip->INODE.i_links_count -= 1;
    pmip->dirty = 1;
    iput(pmip);

    //bdalloc(mip->dev, mip->INODE.i_block[0]);
    //idalloc(mip->dev, mip->ino);
    iput(mip);

    return 0;
}
