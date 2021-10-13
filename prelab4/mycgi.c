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

#define BLKSIZE 4096
#define MAX 10000
typedef struct {
    char *name;
    char *value;
} ENTRY;

ENTRY entry[MAX];

int my_new_ls(char *fname) {
    struct group *grp;
    struct passwd *pwd;
    char *t1 = "wrxwrxwrx-------";
    char *t2 = "----------------";
    struct stat fstat, *sp;
    int r, i;
    char ftime[64];
    sp = &fstat;
    if ( (r = lstat(fname, &fstat)) < 0 ) {
        printf("<p> Can't stat </p>");
        return -1;
    }
    if ((sp->st_mode & 0xF000) == 0x8000)
        printf("<p>%c", '-');
    if ((sp->st_mode & 0xF000) == 0x4000)
        printf("<p>%c", 'd');
    if ((sp->st_mode & 0xF000) == 0xA000)
        printf("<p>%c", 'l');
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
    printf("%4d ", sp->st_size);
   
    strcpy(ftime, ctime(&sp->st_ctim));
    ftime[strlen(ftime)-1] = 0;
    printf("%s ", ftime);
    printf("%s", fname);
    printf("</p>");
    return 0;
}

int my_ls(char *pathname) {
    DIR *dir;
    struct dirent *d;
    dir = opendir(pathname);

    while ((d = readdir(dir)) != NULL) {
        my_new_ls(d->d_name);
    }
    close(dir);
    return 0;
}

int my_mkdir(char cwd[128], char *pathname) {
    if (mkdir(pathname, 0644)) {
        printf("<p> mkdir() failed </p>");
        printf("<p> errno: %s </p>", strerror(errno));
        return -1;
    } else {
        printf("<p> mkdir() success </p>");
        return 0;
    }
}

int my_rmdir(char *pathname) {
    if (rmdir(pathname)) {
        printf("<p> rmdir() failed </p> ");
        printf("<p> errno: %s </p> ", strerror(errno));
        return -1;
    } else {
        printf("<p> rmdir() success </p> ");
        return 0;
    }
}

int my_rm(char *pathname) {
    if (unlink(pathname)) {
        printf("<p> rm() failed </p> ");
        printf("<p> errno: %s </p>", strerror(errno));
        return -1;
    } else {
        printf("<p> rm() success </p> ");
        return 0;
    }
}

int my_cat(char *file) {
    FILE *fp;
    int c;
    if (!strcmp(file, ""))
        return -1;
    if ((fp = fopen(file, "r")) == 0)
        return -1;
    while ((c = fgetc(fp)) != EOF) {
        putchar(c);
    }
    return 0;
}

int my_cp(char *infile, char *outfile) {
    int fd, gd, n, total = 0;
    char buf[BLKSIZE];
    if (!strcmp(infile, "") || !strcmp(outfile, ""))
        return -1;

    if ((fd = (open(infile, O_RDONLY))) < 0)
        return -2;

    if ((gd = open(outfile, O_WRONLY|O_CREAT, 0644)) < 0)
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

int main(int argc, char *argv[]) 
{
  int i, m, r;
  char cwd[128];

  m = getinputs();    // get user inputs name=value into entry[ ]
  getcwd(cwd, 128);   // get CWD pathname

  printf("Content-type: text/html\n\n");
  printf("<p>pid=%d uid=%d cwd=%s\n", getpid(), getuid(), cwd);

  printf("<H1>Echo Your Inputs</H1>");
  printf("You submitted the following name/value pairs:<p>");
 
  for(i=0; i <= m; i++)
     printf("%s = %s<p>", entry[i].name, entry[i].value);
  printf("<p>");

  if (!strcmp(entry[0].value, "ls")) {
    if (!strcmp(entry[1].value, ""))
      my_ls(cwd);
    else {
        char *temp = strcat(cwd, "/");
        strcat(temp, entry[1].value);
        my_ls(temp);
    }
  } else if (!strcmp(entry[0].value, "mkdir")) {
      printf("<p> %d </p>", my_mkdir(cwd, entry[1].value));
  } else if (!strcmp(entry[0].value, "rmdir")) {
      printf("<p> %d </p>", my_rmdir(entry[1].value));
  } else if (!strcmp(entry[0].value, "rm")) {
      printf("<p> %d </p>", my_rm(entry[1].value));
  } else if (!strcmp(entry[0].value, "cat")) {
      printf("<p> %d </p>", my_cat(entry[1].value));
  } else if (!strcmp(entry[0].value, "cp")) {
      printf("<p> %d </p>", my_cp(entry[1].value, entry[2].value));
  } else {
      printf("<p> Enter a valid command </p>");
  }

  // create a FORM webpage for user to submit again 
  printf("</title>");
  printf("</head>");
  printf("<body text=\"#FFFFFF\" bgcolor=\"#000000\" link=\"#330033\" leftmargin=8 topmargin=8");
  
  //------ NOTE : CHANGE ACTION to YOUR login name ----------------------------
  printf("<p>");
  printf("<form METHOD=\"POST\" ACTION=\"http://69.166.48.15/~todd/cgi-bin/mycgi.bin\">");
  
  printf("Enter command : <INPUT NAME=\"command\"> <P>");
  printf("Enter filename1: <INPUT NAME=\"filename1\"> <P>");
  printf("Enter filename2: <INPUT NAME=\"filename2\"> <P>");
  printf("Submit command: <INPUT TYPE=\"submit\" VALUE=\"Click to Submit\"><P>");
  printf("</form>");
  printf("------------------------------------------------<p>");

  printf("</body>");
  printf("</html>");
}

