#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <fcntl.h>
#include <sys/socket.h>

#include "../include/commands.h"

#define BLKSIZE 4096
#define MAX 10000

char *commands_str_server[] = {
    "pwd",
    "cd",
    "mkdir",
    "rm",
    "rmdir",
    "cat",
    "cp",
    "ls",
    "get",
    "put"
};

char *commands_str_client[] = {
    "lpwd",
    "lcd",
    "lmkdir",
    "lrm",
    "lrmdir",
    "lcat",
    "lcp",
    "lls"
};

int (*commands_fn[]) (char **) = {
    &my_pwd,
    &my_cd,
    &my_mkdir,
    &my_rm,
    &my_rmdir,
    &my_cat,
    &my_cp,
    &my_ls,
    &my_get,
    &my_put
};

int num_commands_client() {
    return sizeof(commands_str_client) / sizeof(char *);
}

int num_commands_server() {
    return sizeof(commands_str_server) / sizeof(char *);
}

char *readline() {
    char *newline = NULL;
    size_t bufsize = 0;

    if (getline(&newline, &bufsize, stdin) == -1) {
        if (feof(stdin))
            exit(EXIT_SUCCESS);
        else {
            perror("Readline");
            exit(EXIT_FAILURE);
        }
    }
    return newline;
}

char **parseline(char *line) {
    int bufsize = 256, pos = 0;
    char **tokens = malloc(bufsize * sizeof(char *));
    char *token;

    token = strtok(line, " \t\r\n\a");

    while (token != NULL) {
        tokens[pos] = token;
        pos += 1;
        token = strtok(NULL, " \t\r\n\a");
    }
    tokens[pos] = NULL;
    return tokens;
}

int my_get(char **args) {
    int n;
    char data[256] = {0};
    FILE *fp;
    struct stat stat;
    char *sending = (char *)malloc(32 * sizeof(char));

    lstat(args[1], &stat);
    
    sprintf(sending, "%ld", stat.st_size);

    n = write(cfd, sending, 256);

    fp = fopen(args[1], "r");

    if (fp == NULL) {
        perror("Error reading file");
        free(sending);
        exit(1);
    }
    while (fgets(data, 256, fp) != NULL) {
        if (send(cfd, data, sizeof(data), 0) == -1) {
            perror("Error sending file");
            free(sending);
            exit(1);
        }
        bzero(data, 256);
    }
    free(sending);
    fclose(fp);
    return 0;
}

int my_put(char **args) {
  int q = 0, total = 0;
  FILE *fp;
  char buffer[256];

  fp = fopen(args[2], "w");
  while (total < atoi(args[1])) {
    q = read(cfd, buffer, 256);
    if (q < 0) {
      break;
    }
    fprintf(fp, "%s", buffer);
    total += (strlen(buffer) + 1);
    bzero(buffer, 256);
  }
  fclose(fp);
  return 0;
}

int my_pwd() {
    char wd[128];
    getcwd(wd, 128);
    printf("%s\n", wd);
    return 0;
}

int my_cd(char **args) {
    chdir(args[1]);
    return 0;
}

int my_new_ls(char *fname) {
    struct group *grp;
    struct passwd *pwd;
    char *t1 = "xwrxwrxwr-------";
    char *t2 = "----------------";
    struct stat fstat, *sp;
    int r, i;
    char ftime[64];
    sp = &fstat;
    if ( (r = lstat(fname, &fstat)) < 0 ) {
        printf("Can't stat\n");
        return -1;
    }
    if ((sp->st_mode & 0xF000) == 0x8000)
        printf("%c", '-');
    if ((sp->st_mode & 0xF000) == 0x4000)
        printf("%c", 'd');
    if ((sp->st_mode & 0xF000) == 0xA000)
        printf("%c", 'l');
    for (i = 8; i >= 0; i--) {
        if (sp->st_mode & (1 << i))
            printf("%c", t1[i]);
        else
            printf("%c", t2[i]);
    }
    printf("%4d ", sp->st_nlink);
    if ((pwd = getpwuid(fstat.st_uid)) != NULL)
        printf("%-8.8s ", pwd->pw_name);
    else
        printf("%-8d ", fstat.st_uid);
   
    if ((grp = getgrgid(fstat.st_gid)) != NULL)
        printf(" %-8.8s", grp->gr_name);
    else
        printf(" %-8d", fstat.st_gid);
    printf("%6d ", sp->st_size);
   
    strcpy(ftime, ctime(&sp->st_ctim));
    ftime[strlen(ftime)-1] = 0;
    printf("%s ", ftime);
    printf("%s", fname);
    printf("\n");
    return 0;
}

int my_ls(char **args) {
    DIR *dir;
    struct dirent *d;
    dir = opendir(args[1]);

    while ((d = readdir(dir)) != NULL) {
        my_new_ls(d->d_name);
    }
    closedir(dir);
    return 0;
}

int my_mkdir(char **args) {
    if (args[1] == NULL)
        return -1;
    if (mkdir(args[1], 0655)) {
        printf("mkdir() failed\n");
        printf("errno: %s\n", strerror(errno));
        return -1;
    } else {
        printf("mkdir() success\n");
        return 0;
    }
}

int my_rmdir(char **args) {
    if (args[1] == NULL)
        return -1;
    if (rmdir(args[1])) {
        printf("rmdir() failed\n");
        printf("errno: %s\n", strerror(errno));
        return -1;
    } else {
        printf("rmdir() success\n");
        return 0;
    }
}

int my_rm(char **args) {
    if (args[1] == NULL)
        return -1;
    if (unlink(args[1])) {
        printf("rm() failed\n");
        printf("errno: %s\n", strerror(errno));
        return -1;
    } else {
        printf("rm() success\n");
        return 0;
    }
}

int my_cat(char **args) {
    if (args[1] == NULL)
        return -1;
    FILE *fp;
    int c;
    if (!strcmp(args[1], ""))
        return -1;
    if ((fp = fopen(args[1], "r")) == 0)
        return -1;
    while ((c = fgetc(fp)) != EOF) {
        //putchar(c);
        printf("%c", c);
    }
    return 0;
}

int my_cp(char **args) {
    if (args[1] == NULL || args[2] == NULL)
        return -1;
    int fd, gd, n, total = 0;
    char buf[BLKSIZE];
    if (!strcmp(args[1], "") || !strcmp(args[2], ""))
        return -1;

    if ((fd = (open(args[1], O_RDONLY))) < 0)
        return -2;

    if ((gd = open(args[2], O_WRONLY|O_CREAT, 0655)) < 0)
        return -3;

    while ((n = read(fd, buf, BLKSIZE))) {
        write(gd, buf, n);
        total += n;
    }

    printf("total bytes copied = %d\n", total);
    close(fd);
    close(gd);
    return 0;
}
