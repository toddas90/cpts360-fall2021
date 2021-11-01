#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <ext2fs/ext2fs.h>

#include "../include/mkdir_creat.h"
#include "../include/util.h"
#include "../include/colors.h"
#include "../include/type.h"

extern char pathname[128];
extern int dev, ninodes, imap, bmap;
extern SUPER *sp;
extern GD *gp;

int lab_mkdir() {
    // Check for empty global pathname
    // Do code from book.
    return 0;
}

int lab_creat() {
    // Check for empty global path.
    // Do code in book.
    return 0;
}

int lab_kmkdir(MINODE *pmip, char *basename) {
    return 0;
}

int test_bit(char *buf, int bit) {
    return buf[bit/8] & (1 << (bit % 8));
}

int set_bit(char *buf, int bit) {
    buf[bit/8] |= (1 << (bit % 8));
    return 0;
}

int dec_free_inodes(int dev) {
    char buf[BLKSIZE];

    // I want to check for errors in this, but
    // I'd have to modify KC's get/put_block code
    // in util.c first.

    get_block(dev, 1, buf); // get block into buf
    sp = (SUPER *)buf; // cast buf as SUPER
    sp->s_free_inodes_count -= 1; // decrement inode counter
    put_block(dev, 1, buf); // write buf back

    get_block(dev, 2, buf); // get gd into buf
    gp = (GD *)buf; // cast buf as GD
    gp->bg_free_inodes_count -= 1; // decrement inode count
    put_block(dev, 2, buf); // write buf back
    return 0;
}

int ialloc(int dev) {
    // KC's code from the website, not the book.
    char buf[BLKSIZE];

    get_block(dev, imap, buf); // read inode bitmap into buf

    for (int i = 0; i < ninodes; i++) { // loop through number of inodes
        if (test_bit(buf, i)==0) { // test the bit
            set_bit(buf, i); // Set the bit
            put_block(dev, imap, buf); // write to block
            printf("allocated ino = %d\n", i+1); // bits count from 0; ino from 1
            return i+1;
        }
    }

    return 0;
}

int balloc(int dev) {
    return 0;
}

int ender_child(MINODE *pmip, int ino, char *basename) {
    return 0;
}
