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
#include <libgen.h>     // for dirname()/basename()
#include <time.h> 

#include "../include/commands.h"

#define MAX 4096
#define PORT 1234

char ans[MAX], *line, *cp_line;
int n;

char cwd[128];  

struct sockaddr_in saddr; 
int sfd;

char how[64];
int i;
int tmp;

char **args;

int initialize() {
    printf("1. create a socket\n");
    sfd = socket(AF_INET, SOCK_STREAM, 0); 
    if (sfd < 0) { 
        printf("socket creation failed\n"); 
        exit(0); 
    }
    
    printf("2. fill in server IP and port number\n");
    bzero(&saddr, sizeof(saddr)); 
    saddr.sin_family = AF_INET; 
    //saddr.sin_addr.s_addr = inet_addr("127.0.0.1"); 
    saddr.sin_port = htons(PORT); 
  
    printf("3. connect to server\n");
    if (connect(sfd, (struct sockaddr *)&saddr, sizeof(saddr)) != 0) { 
        printf("connection with the server failed...\n"); 
        exit(0); 
    }
    return 0;
}

int run_command(char **args) {
    if (args[0][0] != 'l' || !strcmp(args[0], "ls"))
        return 1;

    if (!strcmp(args[0], "lls") && args[1] == NULL)
        args[1] = cwd;

    for (int i = 0; i < num_commands_client(); i++) {
        if (strcmp(args[0], commands_str_client[i]) == 0)
            return (*commands_fn[i])(args);
    }
    return 0;
}

int send_command(char *line) {
    n = write(sfd, line, MAX);
    //printf("Client: wrote %d bytes to server\n", n);

    n = read(sfd, ans, MAX);
    //printf("Client: read %d bytes from server\n", n);

    return 0;
}

int main(int argc, char *argv[], char *env[]) 
{
    if (initialize() < 0) {
        printf("Error during initialization\n");
        exit(1);
    }

    printf("********  processing loop  *********\n");
    while (1){
        printf("input a line : ");

        getcwd(cwd, 128); 
        line = readline();

        cp_line = (char*)malloc(sizeof(line));
        strcpy(cp_line, line);

        args = parseline(line);
  
        if ((tmp = run_command(args)) < 0)
            printf("Error running command\n");
        else if (tmp == 1) { 
            if (send_command(cp_line) < 0)
                printf("Error sending command to server\n");
            puts(ans);
        } else
            //puts(ans);
        printf("\n"); 
        free(args);  
        free(cp_line);
  }
}
  
