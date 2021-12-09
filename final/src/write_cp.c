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

    int lbk = 0, blk = 0, startByte = 0, count = 0;
    
    char wbuf[BLKSIZE];
    int ibuf[BLKSIZE/4];
    int tbuf[BLKSIZE/4];

    char ebuf[BLKSIZE];
    bzero(ebuf, BLKSIZE);

    while(nbytes > 0) {
        lbk = oftp->offset / BLKSIZE;
        startByte = oftp->offset % BLKSIZE;

        if (lbk < 12) { // Direct blocks
            if (ip->i_block[lbk] == 0) { //if no data block yet
                ip->i_block[lbk] = balloc(mip->dev); // allocate new block
                ip->i_blocks += 2;
            }

            blk = ip->i_block[lbk];
        } else if (lbk >= 12 && lbk < (12 + 256)) { // Indirect blocks
            if (ip->i_block[12] == 0) {
                ip->i_block[12] = balloc(mip->dev);
                put_block(mip->dev, ip->i_block[12], ebuf);
                ip->i_blocks += 2;
            }

            get_block(mip->dev, ip->i_block[12], ibuf);
            blk = ibuf[lbk - 12];

            if (blk == 0) {
                ibuf[lbk - 12] = balloc(mip->dev);
                put_block(mip->dev, ip->i_block[12], ibuf);
                ip->i_blocks += 2;
            }

            blk = ibuf[lbk-12];
        } else { // Double indirect blocks
            int temp = lbk - (12 + 256);
            int tblk = 0;

            if (ip->i_block[13] == 0) {
                ip->i_block[13] = balloc(mip->dev);
                put_block(mip->dev, ip->i_block[13], ebuf);
                ip->i_blocks += 2;
            }

            get_block(mip->dev, ip->i_block[13], ibuf);
            blk = ibuf[temp / 256];
            tblk = ibuf[temp & 256];

            if (blk == 0) {
                ibuf[temp / 256] = balloc(mip->dev);
                put_block(mip->dev, ibuf[temp / 256], ebuf);
                put_block(mip->dev, ip->i_block[13], ibuf);
                ip->i_blocks += 2;
            }

            get_block(mip->dev, ibuf[temp / 256], tbuf);
            blk = tbuf[temp % 256];

            if (blk == 0) {
                tbuf[temp % 256] = balloc(mip->dev);
                put_block(mip->dev, ibuf[temp / 256], tbuf);
                ip->i_blocks += 2;
            }

            blk = tbuf[temp % 256];
        }

        get_block(mip->dev, blk, wbuf);
        char *cp = wbuf + startByte;
        int remain = BLKSIZE - startByte;

        while (remain) {
            *cp++ = *buf++;
            oftp->offset += 1; 
            count += 1;
            remain -= 1; 
            nbytes -= 1;
            if (oftp->offset > mip->INODE.i_size)
                mip->INODE.i_size += 1;
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
