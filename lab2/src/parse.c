#include <string.h>
#include <stdio.h>

#include "../include/parse.h"

char *parse(char *path) {
    return strtok(path, "/");
}
