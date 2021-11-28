#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <ext2fs/ext2fs.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>

#include "../include/write_cp.h"
#include "../include/open_close_lseek.h"
#include "../include/util.h"
#include "../include/terminal.h"
#include "../include/type.h"

extern int dev, ninodes, nblocks, imap, bmap;
extern SUPER *sp;
extern GD *gp;
extern PROC *running;

int my_write(int fd, char *buf, int nbytes) {
    MINODE *mip = running->fd[fd]->minodePtr;

    OFT *oftp = running->fd[fd];
    char kbuf[BLKSIZE]; 
    int ibuf[256];
    char *cq = buf;

    int count = 0; // Num bytes written
    int lbk = 0, blk = 0, start = 0, remain = 0;
    char *cp;

    while (nbytes) {
        lbk = (oftp->offset / BLKSIZE);
        start = (oftp->offset & BLKSIZE);

        //blk = map(mip->INODE, lbk); // Map function inside util.c is broken despite being the same code.
        if (lbk < 12) { // Direct blocks
            blk = mip->INODE.i_block[lbk];
        } else if (lbk >= 12 && lbk < (12 + 256)) { // Indirect blocks
            get_block(mip->dev, mip->INODE.i_block[12], ibuf);
            blk = ibuf[lbk - 12];
            put_block(mip->dev, mip->INODE.i_block[12], ibuf);
        } else { // Double indirect blocks
            int tmpblk = 0, tmpbuf[256];
            get_block(mip->dev, mip->INODE.i_block[13], ibuf); // get double indirect
            lbk -= (12 + 256);
            tmpblk = ibuf[lbk / 256]; // store block number
            get_block(mip->dev, tmpblk, tmpbuf); // load indirect
            blk = tmpbuf[lbk % 256]; // get physical block
            put_block(mip->dev, tmpblk, tmpbuf); // put back
            put_block(mip->dev, mip->INODE.i_block[13], ibuf); // put back
        }

        get_block(mip->dev, blk, kbuf);
        cp = kbuf + start;
        remain = BLKSIZE - start;

        while (remain) {
            *cp++ = *buf++;
            oftp->offset++; count++;
            remain--; nbytes--;
            if (oftp->offset > mip->INODE.i_size)
                mip->INODE.i_size++;
            if (nbytes <= 0)
                break;
        }

        put_block(mip->dev, blk, kbuf);
    }

    mip->dirty = 1;
    return count;
}

int cat(char *src, char *dest) {
    if (!strcmp(src, "")) {
        printf(YEL "No source file provided\n" RESET);
        return -1;
    } else if (!strcmp(dest, "")) {
        printf(YEL "No destination file provided\n" RESET);
        return -1;
    }

    char mybuf[BLKSIZE], dummy = 0;
    int n = 0;

    int sfd = my_open(src, 0); // Open file for read
    int dfd = my_open(dest, 1); // Open file for read

    while ((n = my_read(sfd, mybuf, BLKSIZE))) {
        mybuf[n] = 0;
        //puts(mybuf);
        my_write(dfd, mybuf, strlen(mybuf));
    }
    my_close(sfd);
    my_close(dfd);
}