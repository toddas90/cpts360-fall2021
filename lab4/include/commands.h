#ifndef COMMANDS_H
#define COMMANDS_H

extern int sfd;
extern int cfd;

extern char *commands_str_client[];
extern char *commands_str_server[];
extern int (*commands_fn[]) (char **);
int num_commands_client();
int num_commands_server();

char *readline();
char **parseline();

int my_pwd();
int my_cd();
int my_ls();
int my_cat();
int my_cp();
int my_mkdir();
int my_rmdir();
int my_rm();
int my_get();
int my_put();



#endif /* COMMANDS_H */
