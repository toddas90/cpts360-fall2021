#include <stdio.h>

typedef unsigned int u32;

char *ctable = "0123456789ABCDEF";

void rpu(u32 x, int base);
void printu(u32 x);
void printd(int x);
void printx(u32 x);
void printo(u32 x);

int main(void) {
    printu(25);
    printd(25);
    printd(-25);
    printx(25);
    printo(25);
    return 0;
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
    printf("%d -> printu() -> ", x);
    (x==0)? putchar('0') : rpu(x, b);
    putchar('\n');
}

void printd(int x) {
    int b = 10; 
    printf("%d -> printd() -> ", x);
    if(x < 0) {
        x = x * -1;
        putchar('-');
        (x == 0) ? putchar('0') : rpu(x, b);
        putchar('\n');
    } else {
        (x == 0) ? putchar('0') : rpu(x, b);
        putchar('\n');
    }
}

void printx(u32 x) {
    int b = 16;
    printf("%d -> printx() -> ", x);
    (x == 0) ? putchar('0') : rpu(x, b);
    putchar('\n');
}

void printo(u32 x) {
    int b = 8;
    printf("%d -> printo() -> ", x);
    (x == 0) ? putchar('0') : rpu(x, b);
    putchar('\n');
}
