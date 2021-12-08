/*********** util.c file ****************/
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <ext2fs/ext2_fs.h>
#include <string.h>
#include <libgen.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include "../include/type.h"
#include "../include/util.h"
#include "../include/terminal.h"

/**** globals defined in main.c file ****/
extern MINODE minode[NMINODE];
extern MINODE *root;
extern PROC   proc[NPROC], *running;
extern MOUNT mountTable[NMOUNT];

extern char gpath[128];
extern char *name[64];
extern int n;

extern int fd, dev, rootdev;
extern int nblocks, ninodes, bmap, imap, iblk;

extern char line[128], cmd[32], pathname[128];

MOUNT *getmptr(int dev) {
    for (int i = 0; i < NMOUNT; i++) {
        if (dev == mountTable[i].dev) {
            return &mountTable[i];
        }
    }
    return 0;
}

int get_block(int dev, int blk, char *buf)
{
    lseek(dev, blk * BLKSIZE, SEEK_SET);
    int n = read(dev, buf, BLKSIZE);
    if (n < 0)
        printf("get_block [%d %d] error \n", dev, blk);
    return 0;
}   

int put_block(int dev, int blk, char *buf)
{
    lseek(dev, blk * BLKSIZE, SEEK_SET);
    int n = write(dev, buf, BLKSIZE);
    if (n != BLKSIZE)
        printf("put_block [%d %d] error\n", dev, blk);
    return 0;
}   

int tokenize(char *pathname)
{
  int i;
  char *s;
  //printf("tokenize %s\n", pathname);

  strcpy(gpath, pathname);   // tokens are in global gpath[ ]
  n = 0;

  s = strtok(gpath, "/");
  while(s){
    name[n] = s;
    n++;
    s = strtok(0, "/");
  }
  name[n] = 0;
  
  //for (i= 0; i<n; i++)
    //printf("%s  ", name[i]);
  //printf("\n");
  return 0;
}

// return minode pointer to loaded INODE
MINODE *iget(int dev, int ino)
{
  int i;
  MINODE *mip;
  MOUNT *mp;
  char buf[BLKSIZE];
  int blk, offset;
  INODE *ip;

    for (i = 0; i<NMOUNT; i++) {
        if(dev == mountTable[i].dev) {
            mp = &mountTable[i];
            break;
        }
    }

  for (i = 0; i<NMINODE; i++) {
        mip = &minode[i];
        if (mip->refCount && mip->dev == dev && mip->ino == ino) {
            mip->refCount++;
           //printf("found [%d %d] as minode[%d] in core\n", dev, ino, i);
           return mip;
        }
    }
    
  for (i = 0; i<NMINODE; i++) {
    mip = &minode[i];
    if (mip->refCount == 0) {
       //printf("allocating NEW minode[%d] for [%d %d]\n", i, dev, ino);
       mip->refCount = 1;
       mip->dev = dev;
       mip->ino = ino;

       // get INODE of ino to buf    
       blk    = (ino-1)/8 + iblk;
       offset = (ino-1) % 8;

       //printf("iget: ino=%d blk=%d offset=%d\n", ino, blk, offset);

       get_block(dev, blk, buf);
       ip = (INODE *)buf + offset;
       // copy INODE to mp->INODE
       mip->INODE = *ip;
       return mip;
    }
  }   
  printf(YEL "PANIC: no more free minodes\n" RESET);
  return 0;
}

void iput(MINODE *mip)
{
    int i, block, offset;
    char buf[BLKSIZE];
    INODE *ip;
    MOUNT *mp;

    if (mip->ino < 2)
        //printf(YEL "Invalid inode\n" RESET);
        return;

    mip->refCount--;
 
    if (mip->refCount > 0) return;
    if (!mip->dirty)       return;

    for (i = 0; i<NMOUNT; i++) {
        if (dev == mountTable[i].dev) {
            mp = &mountTable[i];
            break;
        }
    }

    // Write inode back to disk
    block = (mip->ino - 1) / 8 + iblk;
    offset = (mip->ino - 1) % 8;

    // Get block containing inode
    get_block(mip->dev, block, buf);
    ip = (INODE *)buf + offset; // ip points at INODE
    *ip = mip->INODE; // copy INODE to inode in block
    put_block(mip->dev, block, buf); // write back to disk
    midealloc(mip); // mip->refcount = 0 
}

int test_bit(char *buf, int bit) {
    return buf[bit/8] & (1 << (bit % 8));
}

int set_bit(char *buf, int bit) {
    buf[bit/8] |= (1 << (bit % 8));
    return 0;
}

int clr_bit(char *buf, int bit) {
    buf[bit / 8] &= ~(1 << (bit % 8));
    return 0;
}

int dec_free_inodes(int dev) {
    char buf[BLKSIZE];

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

int inc_free_inodes(int dev) {
    char buf[BLKSIZE];
    get_block(dev, 1, buf);
    sp = (SUPER *)buf;
    sp->s_free_inodes_count += 1;
    put_block(dev, 1, buf);
    get_block(dev, 2, buf);
    gp = (GD *)buf;
    gp->bg_free_inodes_count += 1;
    put_block(dev, 2, buf);
    return 0;
}

int ialloc(int dev)
{
    int i;
    char buf[BLKSIZE];
    MOUNT *mp;

    for (i = 0; i<NMOUNT; i++) {
        if(dev == mountTable[i].dev) {
            mp = &mountTable[i];
            break;
        }
    }

    get_block(dev, mp->imap, buf);

    for  (i=0; i < ninodes; i++) {
        if (test_bit(buf, i) == 0) {
            set_bit(buf,i);
            put_block(dev, mp->imap, buf);
            return i + 1;
        }
    }
    return 0;
}

int balloc(int dev)
{
    int i;
    char buf[BLKSIZE];
    MOUNT *mp;

    for (i = 0; i<NMOUNT; i++) {
        if(dev == mountTable[i].dev) {
            mp = &mountTable[i];
            break;
        }
    }

    get_block(dev,mp->bmap,buf);

    for (i=0; i < nblocks; i++) {
        if(test_bit(buf,i) == 0) {
            set_bit(buf,i);
            put_block(dev,mp->bmap,buf);
            return i + 1;
        }
    }

    return 0;
}

int idalloc(int dev, int ino) {
    int i;
    char buf[BLKSIZE];

    if (ino > ninodes) {
        printf(YEL "inumber out of range\n" RESET);
        return -1;
    }

    MOUNT *mp;

    for (i = 0; i<NMOUNT; i++) {
        if (dev == mountTable[i].dev) {
            mp = &mountTable[i];
            break;
        }
    }

    get_block(dev,mp->imap,buf);
    clr_bit(buf,ino-1);
    //write back
    put_block(dev,mp->imap,buf);
}

int bdalloc(int dev,int bno) {
    int i;
    char buf[BLKSIZE];

    if (bno > nblocks || bno == 0) {
        return -1;
    }

    MOUNT *mp;

    for (i = 0; i<NMOUNT; i++) {
        if (dev == mountTable[i].dev) {
            mp = &mountTable[i];
            break;
        }
    }

    get_block(dev,mp->bmap,buf);
    clr_bit(buf,bno-1);
    put_block(dev, mp->bmap,buf);
}

MINODE *mialloc() {
    for (int i = 0; i < NMINODE; i++) {
        MINODE *mp = &minode[i];
        if (mp->refCount == 0) {
            mp->refCount = 1;
            return mp;
        }
    }
    printf(YEL "Out of MINODES\n" RESET);
    return 0;
}

int midealloc(MINODE *mip) {
    mip->refCount = 0;
    return 0;
}

int search(MINODE *mip, char *name)
{
   int i; 
   char *cp, c, sbuf[BLKSIZE], temp[256];
   DIR *dp;
   INODE *ip;

   //printf("search for %s in MINODE = [%d, %d]\n", name,mip->dev,mip->ino);
   ip = &(mip->INODE);

   /*** search for name in mip's data blocks: ASSUME i_block[0] ONLY ***/

   get_block(dev, ip->i_block[0], sbuf);
   dp = (DIR *)sbuf;
   cp = sbuf;
   //printf("  ino   rlen  nlen  name\n");

   while (cp < sbuf + BLKSIZE){
     strncpy(temp, dp->name, dp->name_len);
     temp[dp->name_len] = 0;
     //printf("%4d  %4d  %4d    %s\n", 
           //dp->inode, dp->rec_len, dp->name_len, dp->name);
     if (strcmp(temp, name)==0){
        //printf("found %s : ino = %d\n", temp, dp->inode);
        return dp->inode;
     }
     cp += dp->rec_len;
     dp = (DIR *)cp;
   }
   return 0;
}

int getino(char *pathname) {
    int i, ino, blk, offset;
    char buf[BLKSIZE];
    INODE *ip;
    MINODE *mip;

    if (strcmp(pathname, "/") == 0) {
        dev = mountTable[0].dev;
        return 2;
    }
  
    // starting mip = root OR CWD
    if (pathname[0]== '/') {
        dev = mountTable[0].dev;
        mip = root;
    } else {
        mip = running->cwd;
        dev = running->cwd->dev;
    }

    mip->refCount += 1; // because we iput(mip) later
    tokenize(pathname);

    for (i=0; i<n; i++) {
        if(!S_ISDIR(mip->INODE.i_mode)) { // Check dir
            printf(YEL "%s is not a directory\n" RESET, name[i]);
            iput(mip);
            return 0;
        }

        ino = search(mip, name[i]);
        
        if (ino == 0) {
            //printf(YEL "name %s does not exist\n" RESET, name[i]);
            iput(mip);
            return 0;
        }

        iput(mip);
        mip = iget(dev, ino); // switch to new minode

        if (mip->mounted == True) {
            dev = mip->mptr->dev;
            iput(mip);
            mip = iget(dev, 2);
            ino = mip->ino;
        }

        if (ino == 2 && mountTable[0].dev != dev && !strcmp(name[i], "..")) {
            for (int j=0; j<NMOUNT; j++) {
                if (mountTable[j].dev == dev) {
                    mip = mountTable[j].mounted_inode;
                    break;
                }
            }

            mip->refCount += 1;
            dev = mip->dev;
            ino = search(mip, name[i]);
        }
    }

    iput(mip);
    return ino;
}

// These 2 functions are needed for pwd()
int findmyname(MINODE *parent, u32 myino, char myname[ ]) 
{
    int i; 
    char *cp, c, sbuf[BLKSIZE], temp[256];
    DIR *dp;
    INODE *ip;

    ip = &(parent->INODE);

    get_block(dev, ip->i_block[0], sbuf);
    dp = (DIR *)sbuf;
    cp = sbuf;

    while (cp < sbuf + BLKSIZE){
        if (myino == dp->inode){ // Compare inode number to inode numbers in parent
            strncpy(myname, dp->name, dp->name_len);
            myname[dp->name_len] = '\0';
            return dp->inode;
        }
        cp += dp->rec_len;
        dp = (DIR *)cp;
    }

    return 0;
}

int findino(MINODE *mip, u32 *myino) // myino = i# of . return i# of ..
{
    DIR *dp;
    char *cp;
    char buf[BLKSIZE];
    
    get_block(mip->dev, mip->INODE.i_block[0], buf);
    dp = (DIR *)buf;
    cp = buf;
    cp += dp->rec_len;
    dp = (DIR*)cp;
    return dp->inode;
}

int ideal_length(char *name) {
    return 4 * ((8 + strlen(name) + 3) / 4);
}

void printblk(MINODE *mip) {
    DIR *dp;
    char *cp;
    char buf[BLKSIZE];

    get_block(dev, mip->INODE.i_block[0], buf);
    dp = (DIR *)buf;
    cp = buf;

    while (cp + dp->rec_len < buf + BLKSIZE) {
        printf("Name: %s\n Name_Len: %d\n Rec_Len: %d\n Inode: %d\n", 
                dp->name, dp->name_len, dp->rec_len, dp->inode);
        cp += dp->rec_len;
        dp = (DIR *)cp;
    }
    printf("Name: %s\n Name_Len: %d\n Rec_Len: %d\n Inode: %d\n", 
                dp->name, dp->name_len, dp->rec_len, dp->inode);
}

int numblks(MINODE *mip) {
    DIR *dp;
    char *cp;
    char buf[BLKSIZE];
    int count= 0;

    get_block(dev, mip->INODE.i_block[0], buf);
    dp = (DIR *)buf;
    cp = buf;

    while (cp + dp->rec_len < buf + BLKSIZE) {
        count += 1;
        cp += dp->rec_len;
        dp = (DIR *)cp;
    }
    return count + 1;
}
