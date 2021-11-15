#ifndef COLOR_H
#define COLOR_H

#define RED   "\x1B[31m"
#define GRN   "\x1B[32m"
#define YEL   "\x1B[33m"
#define BLU   "\x1B[34m"
#define MAG   "\x1B[35m"
#define CYN   "\x1B[36m"
#define WHT   "\x1B[37m"

#define RESET "\x1B[0m"
#define CLR   "\x1b[2J\033[1;1H"

#define BLD   "\x1b[1m"
#define DIM   "\x1b[2m"
#define BLNK  "\x1b[5m"

void clear();

#endif /* COLOR_H */
