#include <stdio.h>
#include <stdlib.h>
#include <ext2fs/ext2_fs.h>
#include <string.h>
#include <libgen.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "../include/type.h"
#include "../include/util.h"
#include "../include/terminal.h"
#include "../include/mount_umount.h"

/**** globals defined in main.c file ****/
extern MINODE minode[NMINODE];
extern MINODE *root;
extern PROC   proc[NPROC], *running;
extern MOUNT mountTable[NMOUNT];

extern char gpath[128];
extern char *name[64];
extern int n;

extern SUPER *sp;
extern GD *gp;
extern INODE *ip;
extern DIR *dp;

extern int fd, dev, rootdev;
extern int nblocks, ninodes, bmap, imap, iblk;

extern char line[128], cmd[32], pathname[128], extra_arg[128];

int mount() {
    if (!strcmp(pathname, "") != !strcmp(extra_arg, "")) { // If only one empty
        printf(YEL "Not enough args\n" RESET);
        return -1;
    }

    if (!strcmp(pathname, "") && !strcmp(extra_arg, "")) { // If both empty
        for (int i = 0; i < NMOUNT; i++) {
            if (mountTable[i].dev != 0) {
                printf("Device [%d]: " BLD "%s" RESET " -> Mount Point: " BLD BLU "%s\n" RESET, 
                        mountTable[i].dev, mountTable[i].name, mountTable[i].mount_name);
            }
        }
        return 0;
    }

    char *path2 = strdup(pathname);
    char *dir = dirname(pathname); // Get dirname
    char *base = basename(path2); // get basename

    for (int i = 0; i < NMOUNT; i++) {
        if (!strcmp(mountTable[i].name, base)) { // If name of thing to mount matches name of mounted thing
            printf(YEL "%s already mounted\n" RESET, base);
            return -1;
        }
    }

    int index = 1;

    for (index; index < NMOUNT; index++) {
        if (mountTable[index].dev == 0) {
            break;
        }
    } 

    char sbuf[BLKSIZE];
    if ((fd = open(base, O_RDWR)) < 0){
        printf(RED "open %s failed\n" RESET, base);
        exit(1);
    }

    /********** read super block  ****************/
    get_block(fd, 1, sbuf);
    sp = (SUPER *)sbuf;

    /* verify it's an ext2 file system ***********/
    if (sp->s_magic != 0xEF53){
        printf(RED "not an EXT2 filesystem!\n" RESET);
        //printf(RED "magic = %x is not an ext2 filesystem\n" RESET, sp->s_magic);
        exit(1);
    }

    int ino  = getino(extra_arg); // get ino:
    MINODE *mip  = iget(dev, ino); // get minode in memory;

    // Check to see if the DIR is mountable
    if (!S_ISDIR(mip->INODE.i_mode)) {
        printf(YEL "Directory required for mount\n" RESET);
        return -1;
    }

    if (!strcmp(extra_arg, running->cwd)) {
        printf(YEL "Directory busy\n" RESET);
        return -1; 
    }

    // Allocating a mountTable entry for the new device
    char buf[BLKSIZE];
    get_block(fd, 2, buf);
    gp = (GD *)buf;

    mountTable[index].dev = fd;
    mountTable[index].bmap = gp->bg_block_bitmap;
    mountTable[index].iblk = gp->bg_inode_table;
    mountTable[index].imap = gp->bg_inode_bitmap;
    mountTable[index].ninodes = sp->s_inodes_count;
    mountTable[index].nblocks = sp->s_blocks_count;
    strcpy(mountTable[index].name, base);
    strcpy(mountTable[index].mount_name, extra_arg);

    // Set minode as mounted on
    mip->mounted = True;
    mip->mptr = &mountTable[index];
    mip->dirty = True;
    mountTable[index].mounted_inode = mip;

    return 0;
}

int umount(char *filesys) {
    char *path2 = strdup(filesys);
    char *dir = dirname(filesys); // Get dirname
    char *base = basename(path2); // get basename

    for (int i = 0; i < NMOUNT; i++) {
        if (!strcmp(mountTable[i].name, base)) { // If name of thing to mount matches name of mounted thing
            printf(YEL "%s already mounted\n" RESET, base);
            return -1;
        }
    }

    // 2. Check whether any file is still active in the mounted filesys;
    //     e.g. someone's CWD or opened files are still there,
    // if so, the mounted filesys is BUSY ==> cannot be umounted yet.
    // HOW to check?      ANS: by checking all minode[].dev

    // 3. Find the mount_point's inode (which should be in memory while it's mounted 
    // on).  Reset it to "not mounted"; then 
    //         iput()   the minode.  (because it was iget()ed during mounting)

    // 4. return 0 for SUCCESS;
}  