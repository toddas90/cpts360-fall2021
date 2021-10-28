#include <stdio.h>
#include <string.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <unistd.h>

#include "../include/util.h" 
#include "../include/type.h"
#include "../include/cd_ls_pwd.h" 

extern PROC *running;
extern MINODE *root;

extern char pathname[128];
extern int dev;

/************* cd_ls_pwd.c file **************/
int cd() {
    int ino = 0;
    
    if (!strcmp(pathname, "")) { // If global path is empty
        ino = 2; // use the root inode
    } else { // If global path is NOT empty
        ino = getino(pathname); // get inode number
         if (ino == 0) {
            fprintf(stderr, "ino = 0\n");
            return -1;
        }
    }

    MINODE *mip = iget(dev, ino); // get minode of path dir

    if (!((mip->INODE.i_mode & 0xF000) == 0x4000)) { // If inode is not a dir, quit
        fprintf(stderr, "INODE is not a dir\n");
        return -1;
    }

    iput(running->cwd);
    running->cwd = mip; // Set running cwd to new minode
    return 0;
}

int ls_file(MINODE *mip, char *name) {
    char *t1 = "xwrxwrxwr-------";
    char *t2 = "----------------";

    struct group *grp; // For group name
    struct passwd *pwd; // for User name
    struct tm ts; // For time
    char ftime[64]; // Formatted time
    time_t file_time = mip->INODE.i_ctime; // time
    ts = *localtime(&file_time); // time

    if ((mip->INODE.i_mode & 0xF000) == 0x8000) // Check if file
        printf("%c", '-');
    if ((mip->INODE.i_mode & 0xF000) == 0x4000) // Check if dir
        printf("%c", 'd');
    if ((mip->INODE.i_mode & 0xF000) == 0xA000) // Check if link
        printf("%c", 'l');
    for (int i = 8; i >= 0; i--) { // Print out permissions
        if (mip->INODE.i_mode & (1 << i))
            printf("%c", t1[i]);
        else
            printf("%c", t2[i]);
    }

    printf("%2d ", mip->INODE.i_links_count); // Print link count

    if ((pwd = getpwuid(mip->INODE.i_uid)) != NULL) // Print name
        printf("%-4.8s ", pwd->pw_name);
    else
        printf("%-4d ", mip->INODE.i_uid);

    if ((grp = getgrgid(mip->INODE.i_gid)) != NULL) // Print group
        printf("%-4.8s", grp->gr_name);
    else
        printf("%-4d", mip->INODE.i_gid);

    printf("%6d ", mip->INODE.i_size); // Print file size

    strftime(ftime, sizeof(ftime), "%b %d %H:%M", &ts); // Format the time
    printf("%s ", ftime); // Print time
    printf("%s\n", name); // Print file name

    return 0;
}

int ls_dir(MINODE *mip) {
    char buf[BLKSIZE], temp[256];
    DIR *dp;
    char *cp;
    MINODE *mmip;
  
    get_block(mip->dev, mip->INODE.i_block[0], buf);
    dp = (DIR *)buf;
    cp = buf;
  
    while (cp < buf + BLKSIZE){
        strncpy(temp, dp->name, dp->name_len);
        temp[dp->name_len] = 0;
        
        mmip = iget(mip->dev, dp->inode); // Child inode
        ls_file(mmip, temp); // Call ls_file with each child node

        cp += dp->rec_len;
        dp = (DIR *)cp;
    }
    printf("\n");
    return 0;
}

int ls() {
    if (!strcmp(pathname, "")) // If path empty, use cwd
        ls_dir(running->cwd);
    else {
        int i = getino(pathname); // get path inode number
        int d = running->cwd->dev; // get device num
        MINODE *m = iget(d, i); // get minode of path dir
        ls_dir(m);
    }
    return 0;
}

void rpwd(MINODE *wd) {
    u32 ino, pino;
    char my_name[64];
    MINODE *pip;

    if (wd == root) // If root, return
        return;
    
    ino = wd->ino; // cwd inode num
    pino = findino(wd, &ino); // cwd parent inode num

    pip = iget(dev, pino); // parent minode
    findmyname(pip, ino, my_name); // get name of cwd from parent
    rpwd(pip); // recursive call, from cwd to root
    printf("/%s", my_name);
    return;
}

void pwd(MINODE *wd) {
    if (wd == root){
        printf("/");
        return;
    } else {
        rpwd(wd);
        printf("\n");
    }
    return;
}

