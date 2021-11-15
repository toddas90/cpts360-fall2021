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
        case 1 : truncate(mip); // Write
            mip->INODE.i_atime = time(NULL);
            mip->INODE.i_mtime = time(NULL);
            oftp->offset = 0;
            breakl
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

int my_lseek(int fd, ) {

}