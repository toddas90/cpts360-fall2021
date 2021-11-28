#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <ext2fs/ext2fs.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>

#include "../include/read_cat.h"
#include "../include/open_close_lseek.h"
#include "../include/util.h"
#include "../include/terminal.h"
#include "../include/type.h"

extern int dev, ninodes, nblocks, imap, bmap;
extern SUPER *sp;
extern GD *gp;
extern PROC *running;

int my_read(int fd, char *buf, int nbytes) {
    MINODE *mip = running->fd[fd]->minodePtr;

    OFT *oftp = running->fd[fd];
    char readbuf[BLKSIZE]; 
    int ibuf[256];
    char *cq = buf;

    int count = 0, lbk = 0, startByte = 0, blk = 0;
    int avil = mip->INODE.i_size - oftp->offset;

    while (nbytes && avil) {
        lbk = oftp->offset / BLKSIZE;
        startByte = oftp->offset % BLKSIZE;

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

        get_block(mip->dev, blk, readbuf);

        /* copy from startByte to buf[ ], at most remain bytes in this block */
        char *cp = readbuf + startByte;   
        int remain = BLKSIZE - startByte;   // number of bytes remain in readbuf[]

        while (remain > 0){
            *cq++ = *cp++;             // copy byte from readbuf[] into buf[]
            oftp->offset += 1;           // advance offset 
            count += 1;                  // inc count as number of bytes read
            avil -= 1; 
            nbytes -= 1;  
            remain -= 1;
            if (nbytes <= 0 || avil <= 0) 
                break;
        }

        put_block(mip->dev, blk, readbuf);
        // if one data block is not enough, loop back to OUTER while for more ...
    }
    //printf("myread: read %d char from file descriptor %d\n", count, fd);  
    return count;   // count is the actual number of bytes read
}

int cat(char *file) {
    if (!strcmp(file, "")) {
        printf(YEL "No file provided\n" RESET);
        return -1;
    }

    if (getino(file) <= 0) {
        printf(YEL "File not found\n" RESET);
        return -1;
    }

    char mybuf[BLKSIZE], dummy = 0;
    int n = 0;

    int fd = my_open(file, 0); // Open file for read

    while ((n = my_read(fd, mybuf, BLKSIZE))) {
        mybuf[n] = 0;
        //puts(mybuf);
        printf("%s", mybuf); // Works but not ideal, will change later.
    }
    my_close(fd);
}
