#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <ext2fs/ext2fs.h>
#include <libgen.h>
#include <sys/stat.h>
#include <time.h>

#include "../include/mkdir_creat.h"
#include "../include/util.h"
#include "../include/colors.h"
#include "../include/type.h"

extern char pathname[128];
extern int dev, ninodes, nblocks, imap, bmap;
extern SUPER *sp;
extern GD *gp;
extern PROC *running;

int lab_mkdir() {
    int ino = 0, bno = 0;
    MINODE *mip;
   
    if (!strcmp(pathname, "")) { // Check if path was provided
        printf("No path provided\n");
        return -1;
    }
    
    char *dir = dirname(pathname); // Get dirname
    char *base = basename(pathname); // get basename
    
    int pino = getino(dir); // Get parent inode number
    MINODE *pmip = iget(dev, pino); // Get parent MINODE

    if (!S_ISDIR(pmip->INODE.i_mode)) { // Check if parent is DIR
        printf("Parent is not a directory\n");
        return -1;
    }
    if (search(pmip, base) != 0) { // Check if basename already exists
        printf("Directory exists\n");
        return -1;
    }
    
    ino = ialloc(dev); // Allocate inode
    bno = balloc(dev); // Allocate disk block
    
    // This chunk gets a new INODE and puts it into the MINODE.
    // It also initializes the INODE to a DIR.
    mip = iget(dev, ino); // Load inode into a MINODE

    mip->INODE.i_mode = 0x41ED; // Set mode as DIR with standard permissions (04755)
    mip->INODE.i_uid = running->uid; // Set uid of INODE
    mip->INODE.i_gid = running->gid; // Set gid of INODE
    mip->INODE.i_size = BLKSIZE; // Set block size
    mip->INODE.i_links_count = 2; // two links, . and ..
    mip->INODE.i_ctime = time(NULL); // Set to current time
    mip->INODE.i_atime = mip->INODE.i_mtime = mip->INODE.i_ctime;
    mip->INODE.i_blocks = 2; // . and ..
    mip->INODE.i_block[0] = bno; // New dir has 1 data block
    for (int j = 1; j < 12; j++) {
        mip->INODE.i_block[j] = 0; // Set blocks 1-14 to 0
    }
    
    // Sets MINODE dirty and writes it to disk.
    mip->dirty = 1; // Set new MINODE to dirty.
    iput(mip); // Write new MINODE to the disk.
    
    // Creates the . and .. entries in the new INODE
    char buf[BLKSIZE]; // Create buf
    bzero(buf, BLKSIZE); // Zero out the buffer
    DIR *dp = (DIR *)buf;
    dp->inode = ino;
    dp->rec_len = 12;
    dp->name_len = 1;
    dp->name[0] = '.';

    dp = (char *)dp + 12; // This line caused me so much pain
    dp->inode = pino;
    dp->rec_len = BLKSIZE - 12;
    dp->name_len = 2;
    dp->name[0] = dp->name[1] = '.';
    put_block(dev, bno, buf); // Write block to disk

    enter_child(pmip, ino, base); // Put child name in parent
    
    pmip->INODE.i_links_count += 1;
    pmip->dirty = 1; // set parent dirty
    iput(pmip); // write to disk
    return 0;
}

int lab_creat() {
    // Check for empty global path.
    // Do code in book.
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
            //printf("allocated ino = %d\n", i+1); // bits count from 0; ino from 1
            return i+1;
        }
    }
    return 0;
}

int balloc(int dev) {
    // Current code is a mirror of
    // ialloc() except it is using
    // the bmap instead of imap.
    char buf[BLKSIZE];

    get_block(dev, bmap, buf);
    
    for (int i = 0; i < nblocks; i++) {
        if (test_bit(buf, i)==0) {
            set_bit(buf, i);
            put_block(dev, bmap, buf);
            //printf("allocated bno = %d\n", i+1);
            return i+1;
        }
    }
    return 0;
}

int enter_child(MINODE *pmip, int ino, char *basename) {
    char buf[BLKSIZE];
    int need_len = 0, remain = 0, i = 0;
    DIR *dp;
    char *cp;

    for (i = 0; i < pmip->INODE.i_size/BLKSIZE; i++) { // KC said to assume 12 in the book?
        if (pmip->INODE.i_block[i] == 0) // If the block doesn't exist, break.
            break;
        get_block(pmip->dev, pmip->INODE.i_block[i], buf); // Read block
        need_len = ideal_length(basename); // Get length needed for new entry
       
        dp = (DIR *)buf;
        cp = buf;
        //printf("First name: %s, len: %d\n", dp->name, dp->rec_len);
        while (cp + dp->rec_len < buf + BLKSIZE) { // Skip to last element
            cp += dp->rec_len;
            dp = (DIR *)cp;
            //printf("Current name: %s, len: %d\n", dp->name, dp->rec_len);
        } // dp now should point to last entry in block.
        remain = dp->rec_len - ideal_length(dp->name); // Get remaining free block space
        if (remain >= need_len) { // If there is enough space for the new item
            dp->rec_len = ideal_length(dp->name); // Shrink the space
            //printf("%s new len: %d\n", dp->name, dp->rec_len);
            cp += dp->rec_len; // Move past item
            dp = (DIR *)cp;
            dp->inode = ino; // Put the new item at the end
            dp->rec_len = remain;
            dp->name_len = strlen(basename);
            strcpy(dp->name, basename);
            //printf("New entry name: %s, len: %d\n", dp->name, dp->rec_len);
            put_block(dev, pmip->INODE.i_block[i], buf); // Write block back to disk.
            return 0;
        } else { // If new dir can't fit in current block
            char nbuf[BLKSIZE];
            int new_bno = balloc(dev);
            pmip->INODE.i_block[i] = new_bno;
            pmip->INODE.i_size += BLKSIZE;
            pmip->dirty = 1;
            get_block(dev, new_bno, nbuf);
            DIR *ndp = (DIR *)buf;
            char *ncp = buf;
            dp->inode = ino;
            dp->rec_len = BLKSIZE;
            dp->name_len = strlen(basename);
            strcpy(dp->name, basename);
            put_block(dev, new_bno, nbuf);
            return 0;
        }
    }
    return 0;
}

int ideal_length(char *name) {
    return 4 * ((8 + strlen(name) + 3) / 4);
}
