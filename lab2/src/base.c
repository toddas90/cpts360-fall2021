#include <stdio.h>             // for I/O
#include <stdlib.h>            // for I/O
#include <libgen.h>            // for dirname()/basename()
#include <string.h>

#include "../include/ls.h"
#include "../include/mkdir.h"
#include "../include/quit.h"
#include "../include/tree.h"
#include "../include/cd.h"
#include "../include/pwd.h"
#include "../include/debug.h"

Node *root, *cwd, *start;
char line[128];
char command[16], pathname[64];

//               0       1      2    
char *cmd[] = {"mkdir", "ls", "cd", "pwd", "creat",
            "rm", "save", "reload", "menu","quit", "debug",NULL};

int findCmd(char *command) {
   int i = 0;
   while(cmd[i]) {
     if (strcmp(command, cmd[i]) == 0)
         return i;
     i++;
   }
   return -1;
}

int main() {
  int index;

  initialize();

  printf("NOTE: commands = [mkdir|ls|cd|pwd|creat|rm|save|reload|menu|quit]\n");

  while(1) {
      printf("Enter command line : ");
      fgets(line, 128, stdin);
      line[strlen(line)-1] = 0;

      command[0] = pathname[0] = 0;
      sscanf(line, "%s %s", command, pathname);
      printf("command=%s pathname=%s\n", 
              command, pathname);
      
      if (command[0] == 0) 
         continue;

      index = findCmd(command);

      switch (index) {
        case 0: mkdir(pathname); 
                break;
        case 1: ls(pathname);            
                break;
        case 2: cd(pathname);
                break;
        case 3: pwd(cwd);
                printf("\n"); break;
        case 4: ;//creat();
                break;
        case 5: ;//rm();
                break;
        case 6: ;//save();
                break;
        case 7: ;//reload();
                break;
        case 8: ;//menu();
                break;
        case 9: quit();          
                break;
        case 10: print_ptr();
                 break;
        default: printf("Invalid Command\n");
      }
  }
}

