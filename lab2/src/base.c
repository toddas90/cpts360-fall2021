#include <stdio.h>             // for I/O
#include <stdlib.h>            // for I/O
#include <libgen.h>            // for dirname()/basename()
#include <string.h>

#include "ls.h"
#include "mkdir.h"
#include "quit.h"
#include "tree.h"

Node *root, *cwd, *start;
char line[128];
char command[16], pathname[64];

//               0       1      2    
char *cmd[] = {"mkdir", "ls", "quit", NULL};

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

  printf("NOTE: commands = [mkdir|ls|quit]\n");

  while(1) {
      printf("Enter command line : ");
      fgets(line, 128, stdin);
      line[strlen(line)-1] = 0;

      command[0] = pathname[0] = 0;
      sscanf(line, "%s %s", command, pathname);
      printf("command=%s pathname=%s\n", command, pathname);
      
      if (command[0] == 0) 
         continue;

      index = findCmd(command);

      switch (index) {
        case 0: mkdir(pathname); break;
        case 1: ls("null");            break;
        case 2: quit();          break;
      }
  }
}

