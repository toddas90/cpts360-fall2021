#include <stdio.h>
#include <string.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>

#include "../include/util.h" 
#include "../include/type.h"
#include "../include/cd_ls_pwd.h" 

extern PROC *running;
extern MINODE *root;

/************* cd_ls_pwd.c file **************/
int cd() {
    printf("cd: under construction READ textbook!!!!\n");

    // READ Chapter 11.7.3 HOW TO chdir
}

int ls_file(MINODE *mip, char *name) {
    char *t1 = "xwrxwrxwr-------";
    char *t2 = "----------------";

    struct group *grp;
    struct passwd *pwd;

    char ftime[64];
  
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

    //strcpy(ftime, ctime(mip->INODE.i_ctime));
    //ftime[strlen(ftime)-1] = 0;
    //printf("%s ", ftime);
    printf("%s\n", name);
}
int ls_dir(MINODE *mip) {

    char buf[BLKSIZE], temp[256];
    DIR *dp;
    char *cp;
  
    get_block(mip->dev, mip->INODE.i_block[0], buf);
    dp = (DIR *)buf;
    cp = buf;
  
    while (cp < buf + BLKSIZE){
        strncpy(temp, dp->name, dp->name_len);
        temp[dp->name_len] = 0;
        //printf("%s  ", temp);

        ls_file(dp, temp);

        cp += dp->rec_len;
        dp = (DIR *)cp;
    }
    printf("\n");
}

int ls() {
    printf("ls: list CWD only! YOU FINISH IT for ls pathname\n");
    ls_dir(running->cwd);
}

char *pwd(MINODE *wd) {
    printf("pwd: READ HOW TO pwd in textbook!!!!\n");
    if (wd == root){
        printf("/\n");
        return "";
    } else {
        printf("T");
    }
    return "";
}

