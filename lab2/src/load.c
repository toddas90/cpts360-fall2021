#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../include/load.h"
#include "../include/tree.h"
#include "../include/mkdir.h"
#include "../include/cd.h"
#include "../include/creat.h"

extern Node *root, *cwd, *start;

void load() {
    char type[4], path[64];
    FILE *fp = fopen("lab2.sav", "r");

    if (fp == NULL) {
        printf("Cannot open file\n");
        return;
    }

    if (root->child != NULL) {
        fclose(fp);
        return;
    }
   
    char *line = NULL;
    size_t len = 0;

    while (getline(&line, &len, fp) != -1) {
        sscanf(line, "%s %s", type, path);
        if (!strcmp(path, "/"))
            continue;
        if (*type == 'D')
            mkdir(path);
        if (*type == 'F')
            creat(path);
        //printf("%s", line);
        //printf("%s %s\n", type, path);
    }

    fclose(fp);
    free(line);

}
