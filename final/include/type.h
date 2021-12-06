/*************** type.h file for LEVEL-1 ****************/
#ifndef TYPE_H
#define TYPE_H

#include <ext2fs/ext2_fs.h>

typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;

typedef struct ext2_super_block SUPER;
typedef struct ext2_group_desc  GD;
typedef struct ext2_inode       INODE;
typedef struct ext2_dir_entry_2 DIR;

extern SUPER *sp;
extern GD    *gp;
extern INODE *ip;
extern DIR   *dp;   

#define FREE        0
#define READY       1

#define True        1
#define False       0

#define BLKSIZE  1024
#define NMINODE   128
#define NPROC       2
#define NMOUNT      8
#define NFD        64

typedef struct mount {
    int dev; // 0 means free
    int ninodes; // From superblock
    int nblocks;
    int bmap; // From GD block
    int imap;
    int iblk;
    struct minode *mounted_inode;
    char name[64]; // device name
    char mount_name[64]; // Mounted DIR pathname
}MOUNT;

typedef struct minode{
  INODE INODE;           // INODE structure on disk
  int dev, ino;          // (dev, ino) of INODE
  int refCount;          // in use count
  int dirty;             // 0 for clean, 1 for modified

  int mounted;           // for level-3
  struct mntable *mptr;  // for level-3
}MINODE;

typedef struct oft {
  int mode;
  int refCount;
  MINODE *minodePtr;
  int offset;
}OFT;

typedef struct proc{
  struct proc *next;
  int          pid;      // process ID  
  int          ppid;
  int          status;
  int          uid;      // user ID
  int          gid;
  MINODE      *cwd;      // CWD directory pointer
  OFT *fd[NFD];
}PROC;

#endif /* TYPE_H */
