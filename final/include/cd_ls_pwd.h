#ifndef CD_LS_PWD_H
#define CD_LS_PWD_H

#include "type.h"

int cd();

int ls_file(MINODE *mip, char *n, char *l);

int ls_dir(MINODE *m);

int ls();

void rpwd(MINODE *w);

void pwd(MINODE *w);

#endif /* CD_LS_PWD_H */
