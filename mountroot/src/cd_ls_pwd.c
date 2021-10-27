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
    
    if (!strcmp(pathname, "")) {
        ino = 2;
    } else {
        ino = getino(pathname);
         if (ino == 0) {
            fprintf(stderr, "ino = 0\n");
            return -1;
        }
    }

    MINODE *mip = iget(dev, ino);

    if (!((mip->INODE.i_mode & 0xF000) == 0x4000)) {
        fprintf(stderr, "INODE is not a dir\n");
        return -1;
    }

    iput(running->cwd);
    running->cwd = mip;
    return 0;
}

int ls_file(MINODE *mip, char *name) {
    char *t1 = "xwrxwrxwr-------";
    char *t2 = "----------------";

    struct group *grp;
    struct passwd *pwd;
    struct tm ts;
    char ftime[64];
    time_t file_time = mip->INODE.i_ctime;
    ts = *localtime(&file_time);

    if ((mip->INODE.i_mode & 0xF000) == 0x8000)
        printf("%c", '-');
    if ((mip->INODE.i_mode & 0xF000) == 0x4000)
        printf("%c", 'd');
    if ((mip->INODE.i_mode & 0xF000) == 0xA000)
        printf("%c", 'l');
    for (int i = 8; i >= 0; i--) {
        if (mip->INODE.i_mode & (1 << i))
            printf("%c", t1[i]);
        else
            printf("%c", t2[i]);
    }

    printf("%2d ", mip->INODE.i_links_count);

    if ((pwd = getpwuid(mip->INODE.i_uid)) != NULL)
        printf("%-4.8s ", pwd->pw_name);
    else
        printf("%-4d ", mip->INODE.i_uid);

    if ((grp = getgrgid(mip->INODE.i_gid)) != NULL)
        printf("%-4.8s", grp->gr_name);
    else
        printf("%-4d", mip->INODE.i_gid);

    printf("%6d ", mip->INODE.i_size);

    strftime(ftime, sizeof(ftime), "%b %d %H:%M", &ts);
    printf("%s ", ftime);
    printf("%s\n", name);

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
        
        mmip = iget(mip->dev, dp->inode);
        ls_file(mmip, temp); 

        cp += dp->rec_len;
        dp = (DIR *)cp;
    }
    printf("\n");
    return 0;
}

int ls() {
    if (!strcmp(pathname, ""))
        ls_dir(running->cwd);
    else {
        int i = getino(pathname);
        int d = running->cwd->dev;
        MINODE *m = iget(d, i);
        ls_dir(m);
    }
    return 0;
}

char *pwd(MINODE *wd) {
    //printf("pwd: READ HOW TO pwd in textbook!!!!\n");
    //if (wd == root){
        //printf("/\n");
        //return "";
    //} else {
        //printf("T");
    //}
    //return "";
    return "";
}

