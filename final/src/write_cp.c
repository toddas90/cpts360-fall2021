#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <ext2fs/ext2fs.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>

#include "../include/write_cp.h"
#include "../include/read_cat.h"
#include "../include/open_close_lseek.h"
#include "../include/util.h"
#include "../include/terminal.h"
#include "../include/type.h"

extern int dev, ninodes, nblocks, imap, bmap;
extern SUPER *sp;
extern GD *gp;
extern PROC *running;

int my_write(int fd, char *buf, int nbytes) {
    OFT *oftp = running->fd[fd];
    MINODE *mip = oftp->minodePtr;
    INODE *ip = &mip->INODE;

    int lbk, blk, startByte, remain, count = 0;
    
    char wbuf[BLKSIZE];
    char indbuf[BLKSIZE/4]; //for indirects
    char tbuf[BLKSIZE/4];

    char ebuf[BLKSIZE];
    bzero(ebuf, BLKSIZE);

    while(nbytes) {
        lbk = oftp->offset / BLKSIZE;
        startByte = oftp->offset % BLKSIZE;

        if (lbk < 12) { // Direct blocks
            if (ip->i_block[lbk] == 0) { //if no data block yet
                ip->i_block[lbk] = balloc(mip->dev);
            }

            blk = mip->INODE.i_block[lbk];
        } else if (lbk >= 12 && lbk < (12 + 256)) { // Indirect blocks
            if (ip->i_block[12] == 0) {
                ip->i_block[12] = balloc(mip->dev);
                put_block(mip->dev, ip->i_block[12], ebuf);
            }

            get_block(mip->dev, ip->i_block[12], indbuf);
            blk = indbuf[lbk-12];

            if (blk == 0) {
                indbuf[lbk-12] = balloc(mip->dev);
                put_block(mip->dev, ip->i_block[12], indbuf);
            }

            blk = indbuf[lbk-12];
        } else { // Double indirect blocks
            if (ip->i_block[13] == 0) {
                ip->i_block[13] = balloc(mip->dev);
                put_block(mip->dev, ip->i_block[13], ebuf);
            }

            get_block(mip->dev, ip->i_block[13], indbuf);
            blk = indbuf[(lbk-(256+12))/256];

            if (blk == 0) {
                indbuf[(lbk-(256+12))/256] = balloc(mip->dev);
                put_block(mip->dev, ip->i_block[(lbk-(256+12))/256], ebuf);
            }

            get_block(mip->dev, indbuf[(lbk-(256+12))/256], tbuf);
            blk = tbuf[lbk-(256+12)%256];

            if (blk == 0) {
                tbuf[lbk-(256+12)%256] = balloc(mip->dev);
                put_block(mip->dev, indbuf[(lbk-(256+12))/256], tbuf);
            }

            blk = tbuf[lbk-(256+12)%256];
        }

        get_block(mip->dev, blk, wbuf);
        char *cp = wbuf + startByte;
        remain = BLKSIZE - startByte;

        while (remain) {
            *cp++ = *buf++;
            oftp->offset++; 
            count++;
            remain--; 
            nbytes--;
            if (oftp->offset > mip->INODE.i_size)
                mip->INODE.i_size++;
            if (nbytes <= 0)
                break;
        }
        put_block(mip->dev, blk, wbuf);
    }

    mip->dirty = 1;
    iput(mip);

    return count;
}

int cp(char *src, char *dest) {
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
    int dfd = my_open(dest, 1); // Open file for write

    while ((n = my_read(sfd, mybuf, BLKSIZE))) {
        mybuf[n] = 0;
        my_write(dfd, mybuf, n);
    }
    my_close(sfd);
    my_close(dfd);
    return 0;
}
