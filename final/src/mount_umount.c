#include <stdio.h>
#include <stdlib.h>
#include <ext2fs/ext2_fs.h>
#include <string.h>

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

int mount() {
    int display = False; // If empty, just print. If full, mount.
    
    if (!strcmp(pathname, "") && !strcmp(extra_arg, "")) { // If both empty
        display = True;
    }

    if (!strcmp(pathname, "") != !strcmp(extra_arg, "")) { // If only one empty
        printf(YEL "Not enough args\n" RESET);
        return -1;
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

    // 2. Check whether filesys is already mounted: 
    // (you may store mounted filesys name in the MOUNT table entry). 
    // If already mounted, reject;
    // else: allocate a free MOUNT table entry (dev=0 means FREE).

    // 3. LINUX open filesys for RW; use its fd number as the new DEV;
    // Check whether it's an EXT2 file system: if not, reject.

    // 4. For mount_point: find its ino, then get its minode:
    //     ino  = getino(pathname);  // get ino:
    //     mip  = iget(dev, ino);    // get minode in memory;    

    // 5. Check mount_point is a DIR.  
    // Check mount_point is NOT busy (e.g. can't be someone's CWD)

    // 6. Allocate a FREE (dev=0) mountTable[] for newdev;

    // Record new DEV, ninodes, nblocks, bmap, imap, iblk in mountTable[] 


    // 7. Mark mount_point's minode as being mounted on and let it point at the
    // MOUNT table entry, which points back to the mount_point minode.

    // return 0 for SUCCESS;
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