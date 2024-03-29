/****************************************************************************
*                   KCW: mount root file system                             *
*****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <ext2fs/ext2_fs.h>
#include <string.h>
#include <libgen.h>
#include <sys/stat.h>
#include <time.h>

#include "../include/type.h"
#include "../include/util.h"
#include "../include/cd_ls_pwd.h"
#include "../include/mkdir_creat.h"
#include "../include/terminal.h"
#include "../include/link_unlink.h"
#include "../include/rmdir.h"
#include "../include/symlink.h"
#include "../include/open_close_lseek.h"
#include "../include/read_cat.h"
#include "../include/write_cp.h"
#include "../include/mount_umount.h"
#include "../include/switch.h"

extern MINODE *iget();

MINODE minode[NMINODE];
MINODE *root;
PROC   proc[NPROC], *running;
MOUNT mountTable[NMOUNT];

char gpath[128]; // global for tokenized components
char *name[64];  // assume at most 64 components in pathname
int   n;         // number of component strings

SUPER *sp;
GD *gp;
INODE *ip;
DIR *dp;

int fd, dev, rootdev;
int nblocks, ninodes, bmap, imap, iblk;
char line[128], cmd[32], pathname[128], extra_arg[128];

void my_banner();
void help();

int init()
{
  int i, j;
  MINODE *mip;
  PROC   *p;
  MOUNT *m;

  for (i=0; i<NMINODE; i++){
    mip = &minode[i];
    mip->dev = mip->ino = 0;
    mip->refCount = 0;
    mip->mounted = 0;
    mip->mptr = 0;
  }
  for (i=0; i<NPROC; i++){
    p = &proc[i];
    p->pid = i;
    p->uid = p->gid = i; // Originally 0
    p->cwd = 0;
    p->status = READY;
    for (int k = 0; k<NFD; k++) {
      p->fd[k] = 0;
    }
  }
  for (i=0; i<NMOUNT; i++) {
      m = &mountTable[i];
      m->dev = 0;
      m->bmap = 0;
      m->iblk = 0;
      m->imap = 0;
      m->nblocks = 0;
      m->ninodes = 0;
  }
  return 0;
}

// load root INODE and set root pointer to it
int mount_root() { 
    mountTable[0].bmap = bmap;
    mountTable[0].dev = dev;
    mountTable[0].iblk = iblk;
    mountTable[0].ninodes = ninodes;
    mountTable[0].nblocks = nblocks;
    mountTable[0].imap = imap;
    strcpy(mountTable[0].name, "root");
    strcpy(mountTable[0].mount_name, "/");
    root = iget(dev, 2);
    mountTable[0].mounted_inode = root;
    return 0;
}

int quit()
{
  int i;
  MINODE *mip;
  for (i=0; i<NMINODE; i++){
    mip = &minode[i];
    if (mip->refCount > 0)
      iput(mip);
  }
  exit(0);
}

char disk[32];
//char *disk = "diskimage";
int main(int argc, char *argv[ ]) {
    int ino;
    char buf[BLKSIZE];

    printf("\n");
    my_banner();

    // Get disk from user
    printf("Enter disk to mount: ");
    fgets(disk, 32, stdin);
    disk[strlen(disk)-1] = 0;
    printf("\n");

    printf("checking EXT2 FS... ");
    if ((fd = open(disk, O_RDWR)) < 0){
        printf(RED "open %s failed\n" RESET, disk);
        exit(1);
    }

    dev = fd;

    /********** read super block  ****************/
    get_block(fd, 1, buf);
    sp = (SUPER *)buf;

    /* verify it's an ext2 file system ***********/
    if (sp->s_magic != 0xEF53){
        printf(RED "not an EXT2 filesystem!\n" RESET);
        //printf(RED "magic = %x is not an ext2 filesystem\n" RESET, sp->s_magic);
        exit(1);
    }      
    printf("OK\n");

    ninodes = sp->s_inodes_count;
    nblocks = sp->s_blocks_count;

    get_block(dev, 2, buf); 
    gp = (GD *)buf;

    bmap = gp->bg_block_bitmap;
    imap = gp->bg_inode_bitmap;
    iblk = gp->bg_inode_table;
    //printf("bmp=%d imap=%d inode_start = %d\n", bmap, imap, iblk);

    printf("initializing....... ");
    init();
    printf("OK\n");

    printf("mounting root...... ");
    mount_root();
    printf("OK\n");

    //printf("root refCount = %d\n", root->refCount);

    //printf("creating P0 as running process\n");
    running = &proc[0];
    running->status = READY;
    running->cwd = iget(dev, 2);
    proc[1].cwd = iget(dev, 2);
    //printf("root refCount = %d\n", root->refCount);

    // WRTIE code here to create P1 as a USER process

    //printf("\n");
    //my_banner();
    printf("\nEnter a command or type \"help\"\n");
 
    while(1){
        printf(BLD MAG " " RESET);
        fgets(line, 128, stdin);
        line[strlen(line)-1] = 0;

        if (line[0]==0)
            continue;
        pathname[0] = 0;

        sscanf(line, "%s %s %s", cmd, pathname, extra_arg);
        printf(RESET);
        //printf("cmd=%s pathname=%s\n", cmd, pathname);
  
        if (strcmp(cmd, "ls")==0)
            ls();
        else if (strcmp(cmd, "cd")==0)
            cd();
        else if (strcmp(cmd, "pwd")==0)
            pwd(running->cwd);
        else if (strcmp(cmd, "mkdir")==0)
            my_mkdir(pathname);
        else if (strcmp(cmd, "rmdir")==0)
            my_rmdir();
        else if (strcmp(cmd, "link")==0)
            my_link();
        else if (strcmp(cmd, "unlink")==0)
            my_unlink();
        else if (strcmp(cmd, "symlink")==0)
            my_symlink();
        else if (strcmp(cmd, "touch")==0 || strcmp(cmd, "creat")==0)
            my_creat(pathname);
        else if (strcmp(cmd, "mount")==0)
            mount();
        else if (strcmp(cmd, "umount")==0)
            umount(pathname);
        else if (strcmp(cmd, "quit")==0)
            quit();
        else if (strcmp(cmd, "clear")==0)
            clear();
        else if (strcmp(cmd, "cat")==0)
            cat(pathname);
        else if (strcmp(cmd, "cp")==0)
            cp(pathname, extra_arg);
        else if (strcmp(cmd, "switch")==0)
            my_switch(pathname);
        else if (strcmp(cmd, "help")==0)
            help();
        printf("\n");
        bzero(pathname, 128);
        bzero(extra_arg, 128);
    }
}

void my_banner() {
    printf(GRN BLD"  ________              __       ______         ________  ______  \n \
/        |            /  |     /      \\       /        |/      \\ \n \
$$$$$$$$/  __    __  _$$ |_   /$$$$$$  |      $$$$$$$$//$$$$$$  | \n \
$$ |__    /  \\  /  |/ $$   |  $$____$$ |      $$ |__   $$ \\__$$/ \n \
$$    |   $$  \\/$$/ $$$$$$/    /    $$/       $$    |  $$      \\ \n \
$$$$$/     $$  $$<    $$ | __ /$$$$$$/        $$$$$/    $$$$$$  | \n \
$$ |_____  /$$$$  \\   $$ |/  |$$ |_____       $$ |     /  \\__$$ | \n \
$$       |/$$/ $$  |  $$  $$/ $$       |      $$ |     $$    $$/ \n \
$$$$$$$$/ $$/   $$/    $$$$/  $$$$$$$$/       $$/       $$$$$$/  \n\n"RESET);
}

void help() {
    printf(BLD BLU "ls" RESET "      - ");
    printf(GRN "Args(1) " RESET "Show file contents\n");

    printf(BLD BLU "cd" RESET "      - ");
    printf(GRN "Args(1) " RESET "Change directory\n");

    printf(BLD BLU "pwd" RESET "     - ");
    printf(GRN "Args(0) " RESET "Print working directory\n");

    printf(BLD BLU "mkdir" RESET "   - ");
    printf(GRN "Args(1) " RESET "Create directory\n");

    printf(BLD BLU "rmdir" RESET "   - ");
    printf(GRN "Args(1) " RESET "Remove directory\n");

    printf(BLD BLU "touch" RESET "   - ");
    printf(GRN "Args(1) " RESET "Create file\n");

    printf(BLD BLU "link" RESET "    - ");
    printf(GRN "Args(2) " RESET "Create hard link\n");

    printf(BLD BLU "unlink" RESET "  - ");
    printf(GRN "Args(1) " RESET "Remove link\n");

    printf(BLD BLU "symlink" RESET " - ");
    printf(GRN "Args(2) " RESET "Create symbolic link\n");

    printf(BLD BLU "cat" RESET "     - ");
    printf(GRN "Args(1) " RESET "Print file to screen\n");

    printf(BLD BLU "cp" RESET "      - ");
    printf(GRN "Args(2) " RESET "Print from src file to dest\n");

    printf(BLD BLU "mount" RESET "   - ");
    printf(GRN "Args(2) " RESET "Mount an ext2 filesystem\n");

    printf(BLD BLU "umount" RESET "  - ");
    printf(GRN "Args(1) " RESET "Unmount an ext2 filesystem\n");

    printf(BLD BLU "switch" RESET "  - ");
    printf(GRN "Args(1) " RESET "Change to a different user process\n");

    printf(BLD BLU "clear" RESET "   - ");
    printf(GRN "Args(0) " RESET "Clear screen\n");

    printf(BLD BLU "help" RESET "    - ");
    printf(GRN "Args(0) " RESET "Display this message\n");

    printf(BLD BLU "quit" RESET "    - ");
    printf(GRN "Args(0) " RESET "Quit\n");
}
