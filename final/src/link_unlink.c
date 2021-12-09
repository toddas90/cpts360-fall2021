#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <ext2fs/ext2fs.h>
#include <sys/stat.h>
#include <libgen.h>

#include "../include/link_unlink.h"
#include "../include/rmdir.h"
#include "../include/mkdir_creat.h"
#include "../include/util.h"
#include "../include/terminal.h"
#include "../include/type.h"

extern char pathname[128], extra_arg[128];
extern int dev;
extern PROC *running;

int my_link() {
    if (!strcmp(pathname, "")) {
        printf(YEL "Source file not found\n" RESET);
        return -1;
    }

    if (!strcmp(extra_arg, "")) {
        printf(YEL "New file not provided\n" RESET);
        return -1;
    }

    int src_ino = getino(pathname);
    MINODE *src_mip = iget(dev, src_ino);

    if (S_ISDIR(src_mip->INODE.i_mode)) {
        printf(YEL "Cannot link a directory\n" RESET);
        return -1;
    }

    if (getino(extra_arg) != 0) {
        printf(YEL "File already exists\n" RESET);
        return -1;
    }

    char *path2 = strdup(extra_arg);
    char *parent = dirname(extra_arg);
    char *child = basename(path2);
    
    int dest_ino = getino(parent);
    MINODE *dest_mip = iget(dev, dest_ino);
    enter_child(dest_mip, src_ino, child);

    src_mip->INODE.i_links_count += 1;
    src_mip->dirty = 1;
    iput(src_mip);
    iput(dest_mip);
    return 0;
}

int my_unlink() {
    //printf("Unlinking %s\n", pathname);

    if (is_owner(pathname) == 0) {
        printf(YEL "Permission denied\n" RESET);
        return -1;
    }

    int ino = getino(pathname);
    MINODE *mip = iget(dev, ino);

    if (S_ISDIR(mip->INODE.i_mode)) {
        printf(YEL "File is a dir\n" RESET);
        return -1;
    }

    char *path2 = strdup(pathname);
    char *parent = dirname(pathname);
    char *child = basename(path2);

    int pino = getino(parent);
    MINODE *pmip = iget(dev, pino);
    rm_child(pmip, child);
    pmip->dirty = True;
    iput(pmip);

    mip->INODE.i_links_count -= 1;
    if (mip->INODE.i_links_count > 0)
        mip->dirty = True;
    else {
        my_truncate(mip);
        //bdalloc(dev, ino);
        idalloc(dev, ino);
    }
    iput(mip);
    return 0;
}

int my_truncate(MINODE *del) {
    int nblk = del->INODE.i_size / BLKSIZE;
    int bno = 0;
    int buf[BLKSIZE];

    if (S_ISDIR(del->INODE.i_mode)) {
        del->dirty = True;

        for (int i = 0; i < 12; i++) { // Direct blocks
            bdalloc(dev, del->INODE.i_block[i]);
        }

        if (nblk >= 12) { // Indirect
            bno = del->INODE.i_block[12];
            get_block(dev, bno, buf);
            for (int i = 0; i < BLKSIZE/4; i++) {
                if (buf[i] == 0)
                    break;
                bdalloc(dev, buf[i]);
            }
            bdalloc(dev, bno);
        }

        if (nblk >= (12 + 256)) { // Double Indirect
            bno = bno = del->INODE.i_block[13];
            get_block(dev, bno, buf);
            for (int i = 0; i < BLKSIZE/4; i++) {
                if (buf[i] == 0)
                    break;
                
                int tmpbuf[BLKSIZE];
                int tmpino = buf[i];
                get_block(dev, tmpino, tmpbuf);
                for (int j = 0; j < BLKSIZE/4; i++) {
                    if (tmpbuf[i] == 0)
                        break;
                    bdalloc(dev, tmpbuf[i]);
                }
                bdalloc(dev, buf[i]);
            }
            bdalloc(dev, bno);
        }
        del->INODE.i_atime = time(NULL);
        del->INODE.i_mtime = time(NULL);
        del->INODE.i_size = 0;
        return 0;
    }
    return -1;
}
