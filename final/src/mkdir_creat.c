#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <ext2fs/ext2fs.h>

#include "../include/mkdir_creat.h"
#include "../include/util.h"
#include "../include/colors.h"
#include "../include/type.h"

extern char pathname[128];

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
    return 0;
}

int ialloc(int dev) {
    
    return 0;
}

int balloc(int dev) {
    return 0;
}

int ender_child(MINODE *pmip, int ino, char *basename) {
    return 0;
}
