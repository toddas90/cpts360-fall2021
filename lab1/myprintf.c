#include <stdio.h>
#include <string.h>
#include <stdarg.h>

typedef unsigned int u32;

void myprintf(char *fmt, ...);
void rpu(u32 i, int b);
void printu(u32 i);
void printd(int i);
void printx(u32 i);
void printo(u32 i);

char *ctable = "0123456789ABCDEF";

int main(int argc, char *argv[], char *env[]) {
    myprintf("cha=%c string=%s      dec=%d hex=%x oct=%o neg=%d\n",
            'A', "this is a test", 100,    100,   100,  -100);
    
    myprintf("argc: %d\n", argc);

    //myprintf("Argv[]: \n");
    for(int i = 0; i < argc; i++) {
        myprintf("[%d]: %s\n", i, argv[i]);
    }
    /*myprintf("Env[]: \n");
    for(int j = 0; env[j] != NULL; j++) {
        myprintf("[%d]: %s\n", j, env[j]);
    }*/

    return 0;
}

void myprintf(char *fmt, ...) {
    va_list valist;
    char *it = fmt;
    char *strval;

    va_start(valist, fmt);
    
    // ADD \n\r detection!!!
    while(*it != '\0') {
        if(*it != '%') {
            putchar(*it);
            it++;
            continue;
        }
        switch(*++it) {
            case 'c':
                putchar(va_arg(valist, int));
                break;
            case 'd':
                printd(va_arg(valist, int));
                break;
            case 'u':
                printu(va_arg(valist, u32));
                break;
            case 'x':
                printx(va_arg(valist, u32));
                break;
            case 'o':
                printo(va_arg(valist, u32));
                break;
            case 's':
                strval = va_arg(valist, char *);
                while(*strval != '\0') {
                    putchar(*strval);
                    strval++;
                }
                break;
        }
        it++;
    }
    va_end(valist);
}

void rpu(u32 x, int base) {
    char c;
    if (x) {
       c = ctable[x % base];
       rpu((x / base), base);
       putchar(c);
    }
}

void printu(u32 x) {
    int b = 10;
    (x==0)? putchar('0') : rpu(x, b);
}

void printd(int x) {
    int b = 10;
    if(x < 0) {
        x = x * -1;
        putchar('-');
        (x == 0) ? putchar('0') : rpu(x, b);
    } else {
        (x == 0) ? putchar('0') : rpu(x, b);
    }
}

void printx(u32 x) {
    int b = 16;
    (x == 0) ? putchar('0') : rpu(x, b);
}

void printo(u32 x) {
    int b = 8;
    (x == 0) ? putchar('0') : rpu(x, b);
}
