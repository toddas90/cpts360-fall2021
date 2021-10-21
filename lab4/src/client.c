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

char ans[MAX], *line;
int n;

char cwd[128];  

struct sockaddr_in saddr; 
int sfd, cfd;

char how[64];
int i;
int tmp;

char **args;

int initialize(char *addr) {
    printf("1. create a socket\n");
    sfd = socket(AF_INET, SOCK_STREAM, 0); 
    if (sfd < 0) { 
        printf("socket creation failed\n"); 
        exit(0); 
    }
    
    printf("2. fill in server IP and port number\n");
    bzero(&saddr, sizeof(saddr)); 
    saddr.sin_family = AF_INET; 
    saddr.sin_addr.s_addr = inet_addr(addr); 
    saddr.sin_port = htons(PORT); 
  
    printf("3. connect to server\n");
    if (connect(sfd, (struct sockaddr *)&saddr, sizeof(saddr)) != 0) { 
        printf("connection with the server failed...\n"); 
        exit(0); 
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

int send_file(char *file, int sockfd) {
    int n;
    char data[256] = {0};
    FILE *fp;
    struct stat stat;
    char *sending = (char *)malloc(128 * sizeof(char));
    
    lstat(file, &stat);
 
    sprintf(sending, "put %ld %s", stat.st_size, file);
   
    n = write(sfd, sending, 256); 
 
    fp = fopen(file, "r");

    if (fp == NULL) {
        perror("Error reading file.");
        free(sending);
        exit(1);
    }
    while(fgets(data, 256, fp) != NULL) {
        if (send(sockfd, data, sizeof(data), 0) == -1) {
            perror("Error sending file.");
            free(sending);
            exit(1);
        }
        bzero(data, 256);  
    }
    free(sending);
    fclose(fp);
    return 0;
}

int recv_file(char **args) {
    int q = 0, total = 0, tmp = 0;
    FILE *fp;
    char buffer[256];
    char *com = (char *)malloc(32 * sizeof(char));
    
    sprintf(com, "get %s", args[1]);
    n = write(sfd, com, 256);
    n = read(sfd, buffer, 256);
    tmp = atoi(buffer);
    fp = fopen(args[1], "w");
    while (total < tmp) {
        fprintf(stderr, "%d < %d\n", total, tmp);
        q = read(sfd, buffer, 256);
        if (q < 0)
            break;
        fprintf(fp, "%s", buffer);
        total += (strlen(buffer)); // +1
        bzero(buffer, 256);
    }
    fclose(fp);
    free(com);
    return 0;
}

int run_command(char **args) {
    if (!strcmp(args[0], "quit")) {
        return -1;
    }

    if (!strcmp(args[0], "put")) {
        send_file(args[1], sfd);
        return 0;
    }

    if (!strcmp(args[0], "get")) {
        recv_file(args);
        return 0;
    }

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

int main(int argc, char *argv[], char *env[]) 
{
    char *addr, *cp_line;
    if (argv[1] == NULL) {
        addr = "127.0.0.1";
    }
    else {
        addr = argv[1];
    }
    if (initialize(addr) < 0) {
        printf("Error during initialization\n");
        exit(1);
    }

    printf("********  processing loop  *********\n");
    while (1){
        printf("input a line : ");

        getcwd(cwd, 128); 
        line = readline(); // LEAK

        cp_line = malloc(strlen(line) + 1); // LEAK
        strcpy(cp_line, line);

        args = parseline(line); // LEAK
  
        if ((tmp = run_command(args)) < 0) {
            //printf("Error running command\n");
            free(args);
            free(line);
            free(cp_line);
            exit(1);
        }
        else if (tmp == 1) { 
            if (send_command(cp_line) < 0)
                printf("Error sending command to server\n");
            puts(ans);
        } else if (tmp == 2) {
            free(args);
            free(line);
            free(cp_line);
            exit(0);
        }
            
        printf("\n"); 
        free(args);  
        free(cp_line);
        free(line);
        bzero(ans, MAX);
  }
}
  
