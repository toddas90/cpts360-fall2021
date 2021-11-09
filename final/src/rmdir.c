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

int my_rmdir() {
    if (!strcmp(pathname, "")) {
        printf(YEL "No path was provided\n" RESET);
        return -1;
    }

    int ino = getino(pathname);
    MINODE *mip = iget(dev, ino);
    
    if (!S_ISDIR(mip->INODE.i_mode)) {
        printf(YEL "Not a directory\n" RESET);
        return -1;
    }

    if (mip->refCount != 1) {
        printf(YEL "Directory busy\n" RESET);
        return -1;
    }

    if (numblks(mip) != 2) {
        printf( YEL "Dir not empty\n" RESET);
        return -1;
    }

    int pino = findino(mip, ino);

    //printf("pino = %d, ino = %d\n", pino, ino); // debug

    MINODE *pmip = iget(mip->dev, pino);
    char name[64];
    findmyname(pmip, ino, name);

    //printf("Name: %s\n", name); // debug

    rm_child(pmip, name);
    //printf("rm_child() = %d\n", rm_child(pmip, name));

    pmip->INODE.i_links_count -= 1;
    pmip->dirty = 1;
    iput(pmip);

    bdalloc(mip->dev, mip->INODE.i_block[0]);
    idalloc(mip->dev, mip->ino);
    iput(mip);

    return 0;
}

int rm_child(MINODE *pmip, char *name) {
    char buf[BLKSIZE];
    DIR *dp, *tdp;
    char *cp;
    int i = 0, temp = 0, size = 0;

    int ino = search(pmip, name);
    if (ino == 0) {
        printf(YEL "Couldn't find child to remove\n" RESET);
        return -1;
    }

    for (i = 0; i < pmip->INODE.i_size/BLKSIZE; i++) {
        if (pmip->INODE.i_block[i] == 0)
            break;
        get_block(dev, pmip->INODE.i_block[i], buf);
        
        dp = (DIR *)buf;
        cp = buf;

        while (cp + dp->rec_len < buf + BLKSIZE) {
            if (dp->inode == ino && dp->rec_len == BLKSIZE) { // If first and only entry in data block
                //size = ((buf + BLKSIZE) - (cp + dp->rec_len));
                idalloc(dev, ino);
                bdalloc(dev, pmip->INODE.i_block[i]); // deallocate block
                pmip->INODE.i_size -= BLKSIZE; // Reduce parent size by BLKSIZE
                if (i < 0 && pmip->INODE.i_block[i+1] != 0) { // if block was between other blocks
                    memcpy(&pmip->INODE.i_block[i], &pmip->INODE.i_block[i+1], BLKSIZE); // shift blocks down 1
                }
                put_block(dev, pmip->INODE.i_block[i], buf);
                return 0;
            } else if (dp->inode == ino) { // entry is first but not only entry, or in middle of block
                size = ((buf + BLKSIZE) - (cp + dp->rec_len));
                temp = dp->rec_len; // store rec_len
                idalloc(dev, ino); // deallocate node
                cp += temp;
                memcpy(dp, cp, size); // move items left
                while (cp + dp->rec_len < buf + BLKSIZE) { // Move to last item
                    cp += dp->rec_len;
                    dp = (DIR *)cp;
                }
                dp->rec_len += temp; // add removed rec_len to end
                put_block(dev, pmip->INODE.i_block[i], buf);
                return 0;
            }
            tdp = dp;
            cp += dp->rec_len;
            dp = (DIR *)cp;
        }
        if (dp->inode == ino) { // if last entry in block
            temp = dp->rec_len; // last item's rec_len
            dp->rec_len = 0; // Remove last item's length
            idalloc(dev, ino);
            dp = tdp; 
            dp->rec_len += temp; // Add old last item's rec len to new last item
            put_block(dev, pmip->INODE.i_block[i], buf);
            return 0;
        }
    }
    put_block(dev, pmip->INODE.i_block[i], buf); // Write block back
    return -1; 
}
