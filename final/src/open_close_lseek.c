#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <ext2fs/ext2fs.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>

#include "../include/open_close_lseek.h"
#include "../include/util.h"
#include "../include/terminal.h"
#include "../include/type.h"

extern int dev, ninodes, nblocks, imap, bmap;
extern SUPER *sp;
extern GD *gp;
extern PROC *running;

int my_open(char *filename, int flags) {
    int ino = getino(filename);
    if (ino == 0) {
        my_creat(filename);
        ino = getino(filename);
    }
    MINODE *mip = iget(dev, ino);

    if (!S_ISREG(mip->INODE.i_mode)) {
        printf(RED "Not a file\n" RESET);
        return  -1;
    }

    if (access(filename, R_OK) != 0) {
        printf(RED "Permission denied\n" RESET);
        return -1;
    }

    OFT *oftp;
    oftp->mode = flags;
    oftp->minodePtr = mip;
    oftp->refCount = 1;
    oftp->offset = 0;

    switch(flags) {
        case 0 : oftp->offset = 0; // Read
            mip->INODE.i_atime = time(NULL);
            break;
        case 1 : my_truncate(mip); // Write
            mip->INODE.i_atime = time(NULL);
            mip->INODE.i_mtime = time(NULL);
            oftp->offset = 0;
            break;
        case 2 : oftp->offset = 0; // RW
            mip->INODE.i_atime = time(NULL);
            mip->INODE.i_mtime = time(NULL);
            break;
        case 3 : oftp->offset = mip->INODE.i_size; // Append
            mip->INODE.i_atime = time(NULL);
            mip->INODE.i_mtime = time(NULL);
            break;
        default : printf(RED "Invalid mode\n" RESET);
            return -1;
    }

    for (int i = 0; i<NFD; i++) {
        if (running->fd[i] == 0) {
            running->fd[i] = &oftp;
            return i;
        }
    }

    mip->dirty = 1;

    return -1;
}

int my_truncate(MINODE *mip) {
    for (int i = 0; i < mip->INODE.i_size/BLKSIZE; i++) {
        if (mip->INODE.i_block[i] == 0)
            continue;
        else {
            bdalloc(dev, mip->INODE.i_block[i]);
        }
    }
    mip->INODE.i_atime = time(NULL);
    mip->INODE.i_mtime = time(NULL);
    mip->INODE.i_size = 0;
    mip->dirty = 1;
    return 0;
}

int close_file(int fd)
{
    if (fd > 64 || fd < 0) {
        printf(RED "fd out of range\n" RESET);
        return -1;
    }

    if (running->fd[fd] == NULL) {
        printf(RED "fd not a valid file descriptor\n" RESET);
        return -1;
    }

    OFT *oftp = running->fd[fd];
    running->fd[fd] = NULL;
    oftp->refCount -= 1;
    if (oftp->refCount > 0)
        return;

    MINODE *mip = oftp->minodePtr;
    iput(mip);
    return 0;
}

int my_lseek(int fd, int pos) {
/*
  From fd, find the OFT entry. 

  change OFT entry's offset to position but make sure NOT to over run either end
  of the file.

  return originalPosition
*/
}

void print_fd() {
    OFT *temp;
    printf("fd     mode     offset     INODE\n");
    printf("--     ----     ------     -----\n");
    for (int i = 0; i<NFD; i++) {
        temp = running->fd[i];
        if (temp == 0)
            continue;
        printf(" %d   %d    %d   [%d, %d]\n", i, temp->mode, temp->offset, temp->minodePtr->dev, temp->minodePtr->ino);
    }
}