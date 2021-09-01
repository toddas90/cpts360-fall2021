#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>

typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;

struct partition {
    u8 drive;             // drive number FD=0, HD=0x80, etc.

    u8 head;             // starting head
    u8 sector;           // starting sector
    u8 cylinder;         // starting cylinder

    u8 sys_type;         // partition type: NTFS, LINUX, etc.

    u8 end_head;         // end head
    u8 end_sector;       // end sector
    u8 end_cylinder;     // end cylinder

    u32 start_sector;     // starting sector counting from 0
    u32 nr_sectors;       // number of of sectors in partition
};
char *dev = "vdisk";
int fd;

    // read a disk sector into char buf[512]
 void read_sector(int fd, int sector, char *buf) { // Originally int, not void.
    lseek(fd, sector*512, SEEK_SET);  // lssek to byte sector*512
    read(fd, buf, 512);               // read 512 bytes into buf[ ]
}

void print_part_info(struct partition *p, char *name, int offset) {
    printf("Partition %s:\n", name);
    printf("    start_sector: %d\n", p->start_sector + offset);
    printf("    end_sector: %d\n", ((p->start_sector + p->nr_sectors) - 1) + offset);
    printf("    nr_sectors: %d\n", p->nr_sectors);
    printf("    sys_type: %x\n", p->sys_type);
}

int main() {
    struct partition *p;
    char buf[512];

    fd = open(dev, O_RDONLY);   // open dev for READ
    read_sector(fd, 0, buf);    // call read_sector()

    p = (struct partition *)(&buf[0x1be]); // p->P1

    // print P1's start_sector, nr_sectors, sys_type);
    
    print_part_info(p, "P1", 0);
    p++;
    print_part_info(p, "P2", 0);
    p++;
    print_part_info(p, "P3", 0);
    p++;
    print_part_info(p, "P4", 0);

    // Write code to print all 4 partitions;
    // ASSUME P4 is EXTEND type, p->P4;
    //printf("P4 start_sector = %d\n", p->start_sector);
    int extstart = p->start_sector;
    read_sector(fd, extstart, buf);

    p = (struct partition *)&buf[0x1BE];    // p->localMBR
    
    int nextstart = extstart + p->nr_sectors;
    //printf("FIRST entry of localMBR\n");
    //printf("start_sector=%d, nsectors=%d\n", p->start_sector, p->nr_sectors);
    // Write YOUR code to get 2nd entry, which points to the next localMBR, etc
    print_part_info(p, "P5", extstart);
    p++;
    
    read_sector(fd, p->start_sector + extstart, buf);
    p = (struct partition *)&buf[0x1BE];

    print_part_info(p, "P6", p->start_sector + nextstart);

    nextstart = nextstart + p->nr_sectors + p->start_sector;

    p++;
    read_sector(fd, p->start_sector + extstart, buf);
    p = (struct partition *)&buf[0x1BE];
    print_part_info(p, "P7", p->start_sector + nextstart);
}
// FIGURE OUT THE START AND END VALUES
