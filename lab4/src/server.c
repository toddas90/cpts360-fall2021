#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <netdb.h> 
#include <netinet/in.h> 
#include <sys/socket.h> 
#include <sys/types.h> 
#include <arpa/inet.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <libgen.h>
#include <time.h>
#include <signal.h>

#include "../include/commands.h"

#define MAX 4096
#define PORT 1234

int n;
int sfd, cfd;
char ans[MAX];
char line[MAX];
char **args;
char cwd[128];

int run_commands(char **args) {
    if (args[0] == NULL)
        return -1;
    
    if (!strcmp(args[0], "ls") && args[1] == NULL)
        args[1] = cwd;

    for (int i = 0; i < num_commands_server(); i++) {
        if (strcmp(args[0], commands_str_server[i]) == 0)
            return (*commands_fn[i])(args);
    }
    return 0;
}

int main() 
{
    getcwd(cwd, 128);
    strcat(cwd, "/root");
    chdir(cwd);
    //if (chroot(cwd) != 0) {
        //perror("chroot root");
        //return 1;
    //}
    int len; 
    struct sockaddr_in saddr, caddr; 
    int i, length;
    printf("1. create a socket\n");
    sfd = socket(AF_INET, SOCK_STREAM, 0); 
    if (sfd < 0) { 
        printf("socket creation failed\n"); 
        exit(0); 
    }
    
    printf("2. fill in server IP and port number\n");
    bzero(&saddr, sizeof(saddr)); 
    saddr.sin_family = AF_INET; 
    saddr.sin_addr.s_addr = htonl(INADDR_ANY); 
    //saddr.sin_addr.s_addr = inet_addr("127.0.0.1"); 
    saddr.sin_port = htons(PORT);
    
    printf("3. bind socket to server\n");
    if ((bind(sfd, (struct sockaddr *)&saddr, sizeof(saddr))) != 0) { 
        printf("socket bind failed\n"); 
        exit(0);  
    }
      
    // Now server is ready to listen and verification 
    if ((listen(sfd, 5)) != 0) { 
        printf("Listen failed\n"); 
        exit(0); 
    }
    while(1){
       // Try to accept a client connection as descriptor newsock
       length = sizeof(caddr);
       cfd = accept(sfd, (struct sockaddr *)&caddr, (socklen_t *)&length);
       if (cfd < 0){
          printf("server: accept error\n");
          exit(1);
       }

       printf("server: accepted a client connection from\n");
       printf("-----------------------------------------------\n");
       printf("    IP=%s  port=%d\n", inet_ntoa(caddr.sin_addr), ntohs(caddr.sin_port));
       printf("-----------------------------------------------\n");
    
       if (!fork()) {
           close(sfd);
           // Processing loop
           while(1){
               printf("server ready for next request ....\n");
               n = read(cfd, line, MAX);
               if (n==0){
                   printf("server: client died, server loops\n");
                   close(cfd);
                   break;
               }
 
           // show the line string
           printf("server: read  n=%d bytes; line = %s\n", n, line);
        
           printf("Changing output to /dev/null\n");
           freopen("/dev/null", "a", stdout);
           setbuf(stdout, ans);
           getcwd(cwd, 128);
           args = parseline(line);
           
           if (!strcmp(args[0], "stop")) {
               //free(args);
               //_exit(0);
               kill(getppid(), SIGTERM);
               exit(1);
           }

           run_commands(args);   
           n = write(cfd, ans, MAX);

           freopen("/dev/tty", "a", stdout);
           printf("Output changed back to /dev/tty\n");   
  
           printf("server: wrote n=%d bytes; line = %s\n", n, ans);
           printf("server: ready for next request\n");
           free(args);
           bzero(ans, MAX);
           }  
       }
       close(cfd);
    }
}
     
