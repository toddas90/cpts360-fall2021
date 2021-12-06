#ifndef MOUNT_UMOUNT_H
#define MOUNT_UMOUNT_H

int mount();
int umount(char *filesys);
int checkfs(char *disk, int index);

#endif