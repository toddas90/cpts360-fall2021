#include <stdio.h>

#include "../include/save.h"
#include "../include/tree.h"
#include "../include/mkdir.h"
#include "../include/cd.h"
#include "../include/creat.h"
#include "../include/pwd.h"

extern Node *root, *start, *cwd;

FILE *fp;

int save() {
    fp = fopen("lab2.sav", "w+");

    save_helper(root);

    fclose(fp);
    return 0;
}

void save_helper(Node *cur) {
    fprintf(fp, "%c ", cur->type);
    save_pwd(cur);
    fprintf(fp, "\n");
    if (cur->child != NULL)
        save_helper(cur->child);
    if (cur->sibling != NULL)
        save_helper(cur->sibling);
}

void save_pwd(Node *node) {
    if (node == root) {
        fprintf(fp, "%s", node->name);
        return;
    }
    save_pwd(node->parent);
    fprintf(fp, "%s/", node->name);
}
