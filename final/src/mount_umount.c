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

extern int fd, dev;
extern int nblocks, ninodes, bmap, imap, iblk;

extern char line[128], cmd[32], pathname[128], extra_arg[128];

int checkfs(char *disk) {
    printf("checking EXT2 FS .... ");
    int nfd = 0;
    char buf[BLKSIZE];
    if ((nfd = open(disk, O_RDWR)) < 0){
        printf(RED "open %s failed\n" RESET, disk);
        exit(1);
    }

    /********** read super block  ****************/
    get_block(nfd, 1, buf);
    sp = (SUPER *)buf;

    /* verify it's an ext2 file system ***********/
    if (sp->s_magic != 0xEF53){
        printf(RED "not an EXT2 filesystem!\n" RESET);
        //printf(RED "magic = %x is not an ext2 filesystem\n" RESET, sp->s_magic);
        exit(1);
    }      
    printf("OK\n");
    return nfd;
}

int mount() {
    int display = False; // If empty, just print. If full, mount.

    if (!strcmp(pathname, "") != !strcmp(extra_arg, "")) { // If only one empty
        printf(YEL "Not enough args\n" RESET);
        return -1;
    }

    if (!strcmp(pathname, "") && !strcmp(extra_arg, "")) { // If both empty
        display = True;
    }
    
    if (display == True) { // If no parameters, print the mountTable values
        for (int i = 0; i < NMOUNT; i++) {
            if (mountTable[i].dev != 0) {
                printf("Device: " BLD "%s" RESET " -> Mount Point: " BLD BLU "%s\n" RESET, 
                        mountTable[i].name, mountTable[i].mount_name);
            }
        }
        return 0;
    }

    char *path2 = strdup(pathname);
    char *dir = dirname(pathname); // Get dirname
    char *base = basename(path2); // get basename

    int index = 0;
    int nfd = 0;

    for (index; index < NMOUNT; index++) {
        if (mountTable[index].dev == 0)
            nfd = checkfs(base); // Check to see if ext2 fs
            printf("Back from checkfs, nfd = %d\n", nfd);
            break;
        if (!strcmp(mountTable[index].name, base)) { // If name of thing to mount matches name of mounted thing
            printf(YEL "%s already mounted\n" RESET, base);
            return -1;
        }
    }

    int ino  = getino(extra_arg);  // get ino:
    MINODE *mip  = iget(dev, ino);    // get minode in memory;

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
    get_block(nfd, 2, buf); // Hanging because nfd is 0?
    gp = (GD *)buf;

    mountTable[index].dev = nfd;
    mountTable[index].mounted_inode = mip;
    mountTable[index].bmap = gp->bg_block_bitmap;
    mountTable[index].iblk = gp->bg_inode_table;
    mountTable[index].ninodes = ninodes;
    mountTable[index].nblocks = nblocks;
    mountTable[index].imap = gp->bg_inode_bitmap;
    strcpy(mountTable[index].name, base);
    strcpy(mountTable[index].mount_name, extra_arg);

    // Set minode as mounted on
    mountTable[index].mounted_inode->mounted = True;
    mountTable[index].mounted_inode->mptr = &mountTable[index];

    return 0;
}

int umount(char *filesys) {
    // 1. Search the MOUNT table to check filesys is indeed mounted.

    // 2. Check whether any file is still active in the mounted filesys;
    //     e.g. someone's CWD or opened files are still there,
    // if so, the mounted filesys is BUSY ==> cannot be umounted yet.
    // HOW to check?      ANS: by checking all minode[].dev

    // 3. Find the mount_point's inode (which should be in memory while it's mounted 
    // on).  Reset it to "not mounted"; then 
    //         iput()   the minode.  (because it was iget()ed during mounting)

    // 4. return 0 for SUCCESS;
}  

//                   IMPLICATIONS of mount:

// Although it is easy to implement mount and umount, there are implications.
				       
// With mount, you must modify the getino(pathname) function to support 
// "cross mount points". Assume that a file system, newfs, has been mounted on 
// the directory /a/b/c/.  When traversing a pathname, crossing mount point may
// occur in both directions.
 
// (3).1. Dwonward traversal: When traversing the pathname /a/b/c/x/y, once you 
// reach the minode of /a/b/c, you should see that the minode has been mounted on 
// (mounted flag=1). Instead of searching for x in the INODE of /a/b/c, you must

//     .Follow the minode's mountTable pointer to locate the mount table entry.
//     .From the newfs's DEV number, iget() its root (ino=2) INODE into memory.
//     .Then, continue to search for x/y under the root INODE of newfs.

// (3).2. Upward traversal: Assume that you are at the directory /a/b/c/x/ and 
// traversing upward, e.g. cd  ../../,  which will cross the mount point at /a/b/c.

// When you reach the root INODE of the mounted file system, you should see it
// is a root directory (ino=2) but its DEV number differs from the real root.
// Using its DEV number, you can locate its mount table entry, which points 
// to the mounted minode of /a/b/c/. Then, you switch to the minode of /a/b/c/ and 
// continue the upward traversal.

// Thus, crossing mount point is like a monkey or squirrel hoping from one tree to
//       another tree and then back.

// (3).3. While traversing a pathname, the dev number may change. You must modify
// the getino(pathname) function to change the (global) dev number whenever it 
// crosses a mount point. Thus,

//         (global) int dev;   // start from root dev OR running->cwd's dev
//         int ino = getino(pathname);

// essentially returns the (dev, ino) of a pathname.

//                   Other Modifications:

// In ialloc()/idealloc(), balloc()/bdealloc(): 
//    must use current dev's imap, bmap, ninodes, nblocks
// 						      .

// In iget()/iput():
//    must use current dev's iblk (inodes_start_block).