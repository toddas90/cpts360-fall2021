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

// Prints the partition info
void print_partition(struct partition *p, int num, int offset) {
    printf("%s%d     %4d   %4d      %4d   %4dK    %2x\n", 
            dev, num, p->start_sector + offset,
            ((p->start_sector + p->nr_sectors) - 1) + offset, 
            p->nr_sectors, (p->nr_sectors / 2), p->sys_type);
}

int main() {
    struct partition *p;
    char buf[512];

    fd = open(dev, O_RDONLY);   // open dev for READ
    read_sector(fd, 0, buf);    // call read_sector()

    p = (struct partition *)(&buf[0x1be]); // p->P1
    
    printf("===============================================\n");
    printf("=   Andrew's Totally Awesome Fdisk -l Clone   =\n");
    printf("===============================================\n");
    printf("Disk      Start    End   Sectors    Size    Id\n");
    // print P1's start_sector, nr_sectors, sys_type);
    
    print_partition(p, 1, 0);
    p++; // incrememnt p1 -> p2
    print_partition(p, 2, 0);
    p++; // increment p2 -> p3, etc...
    print_partition(p, 3, 0);
    p++;
    print_partition(p, 4, 0);
    // Write code to print all 4 partitions;
    // ASSUME P4 is EXTEND type, p->P4;
    
    int extstart = p->start_sector; // Setting extstart to p4's start sector

    if (p->sys_type == 5) { // If P4 is extended
        int extstart = p->start_sector; // Setting extstart to P4's start
        read_sector(fd, extstart, buf); //Read the sector
        p = (struct partition *)&buf[0x1BE];    // p->localMBR
        int nextstart = extstart + p->nr_sectors; // Offset for next start/end values
        print_partition(p, 5, extstart);
        p++; // Increment
    
        int epnum = 6; // Next partition number
        while(p->sys_type != 0) {
            read_sector(fd, p->start_sector + extstart, buf); // Read next sector, offset from P4
            p = (struct partition *)&buf[0x1BE];
            print_partition(p, epnum, p->start_sector + nextstart);
            nextstart = nextstart + p->nr_sectors + p->start_sector; // Set new offset for next partition
            epnum++; // Increment parttion counter
            p++; // Increment partition
        }
    }
}
