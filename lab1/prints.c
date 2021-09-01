#include <stdio.h>
#include <string.h>

void prints(char *s);

int main(int argc, char *argv[]) {
    if(argc < 2) {
        return 1;
    }
    char *c = argv[1];
    // printf("%s\n", c); // For testing
    prints(c);
    return 0;
}

void prints(char *s) {
    while(*s != '\0') {
        putchar(*s);
        s++;
    }
}
