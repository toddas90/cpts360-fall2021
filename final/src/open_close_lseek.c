#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <ext2fs/ext2fs.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>

#include "../include/open_close_lseek.h"
#include "../include/link_unlink.h"
#include "../include/mkdir_creat.h"
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

    //printf(GRN "Open: ino = %d\n" RESET, ino);

    MINODE *mip = iget(dev, ino);

    if (!S_ISREG(mip->INODE.i_mode)) {
        printf(RED "Not a file\n" RESET);
        return  -1;
    }

    OFT *oftp = (OFT *)malloc(sizeof(OFT)); // NEED TO FREE THIS
    oftp->minodePtr = mip;
    oftp->refCount = 1;

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
            running->fd[i] = oftp;
            //printf(GRN "Open: returning fd %d\n" RESET, i);
            return i;
        }
    }
    mip->dirty = 1;
    return -1;
}

int my_close(int fd) {
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
        return 0;

    MINODE *mip = oftp->minodePtr;
    iput(mip);
    return 0;
}

int my_lseek(int fd, int pos) {
    if (fd > 64 || fd < 0) {
        printf(RED "fd out of range\n" RESET);
        return -1;
    }

    if (running->fd[fd] == NULL) {
        printf(RED "fd not a valid file descriptor\n" RESET);
        return -1;
    }

    OFT *oftp = running->fd[fd];

    if (oftp->minodePtr->INODE.i_size < pos || pos < 0) {
        printf(YEL "Pos greater than file size\n" RESET);
        return -1;
    }

    oftp->offset = pos;

    return pos;
}

void print_fd() {
    OFT *temp;
    printf("fd     mode     offset     INODE\n");
    printf("--     ----     ------     -----\n");
    for (int i = 0; i<NFD; i++) {
        temp = running->fd[i];
        if (temp == 0) {
            if (i == 63)
                printf(" 0      0         0       [0, 0]\n");
            continue;
        }
        printf(" %d      %d         %d      [%d, %d]\n", i, temp->mode, temp->offset, temp->minodePtr->dev, temp->minodePtr->ino);
    }
    printf("--------------------------------\n");
}
