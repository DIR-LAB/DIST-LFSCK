/*

Program to display the contents of used inodes on disk using the ext4 FS.






*/
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ext2fs/ext2_fs.h>
#include <ext4.h>
#define BASE_OFFSET 1024                   /* locates beginning of the super block (first group) */
#define FD_DEVICE "/dev/sdb1"               /* the floppy disk device */
#define BLOCK_OFFSET(block) ((block-1)*block_size)

static unsigned int block_size = 0;        /* block size (to be calculated) */

static void read_inode(fd, inode_no, group, inode)
      int                           fd;        /* the floppy disk file descriptor */
      int                           inode_no;  /* the inode number to read  */
      const struct ext4_group_desc *group;     /* the block group to which the inode belongs */
      struct ext4_inode            *inode;     /* where to put the inode from disk  */
{
         lseek(fd, BLOCK_OFFSET(group->bg_inode_table)+(inode_no-1)*sizeof(struct ext4_inode), SEEK_SET);
         read(fd, inode, sizeof(struct ext4_inode));
}
 

int main(void)
{
	struct ext4_super_block super;
	struct ext4_group_desc group;
	int fd;
	unsigned char *bitmap=0;
	struct ext4_inode *test_inode;
	/* open floppy device */

	if ((fd = open(FD_DEVICE, O_RDONLY)) < 0) {
		perror(FD_DEVICE);
		exit(1);  /* error while opening the floppy device */
	}

	/* read super-block */

	lseek(fd, BASE_OFFSET, SEEK_SET); 
	read(fd, &super, sizeof(super));

	if (super.s_magic != EXT4_SUPER_MAGIC) {
		fprintf(stderr, "Not a Ext4 filesystem\n");
		exit(1);
	} 
		
	block_size = 1024 << super.s_log_block_size;

	/* read group descriptor */

	lseek(fd,block_size, SEEK_SET);
	read(fd, &group, sizeof(group));

	bitmap = (unsigned char *)malloc(block_size);    /* allocate memory for the bitmap */
	lseek(fd, BLOCK_OFFSET(group.bg_block_bitmap), SEEK_SET);
	read(fd, bitmap, block_size);   /* read bitmap from disk */


/* Reading the Block Bitmap from the group descriptor */
printf("Block bitmap: ");
    int i, j;
    for (i = 0; i < 10; i++) {
        for (j = 0; j < 8; j++) {
            printf("%d", *(bitmap)>>j & 1);
        }
	bitmap++;
        printf(" \n");
    }

/*Reading the Inodes from the Inode table using the group descriptor */
lseek(fd,BLOCK_OFFSET(group.bg_inode_table),SEEK_SET);
read(fd,test_inode,256);

printf("Inode Information\n ");
printf("Inode Size :%u\n"
       "Inode owner ID :%hu\n"
       "File mode: %hu\n",
	test_inode->i_size,
	test_inode->i_uid,
	test_inode->i_mode);


close(fd);	
/*Printing the Group descriptor */		
	printf("Reading first group-descriptor from device " FD_DEVICE ":\n"
	       "Blocks bitmap block: %u\n"
	       "Inodes bitmap block: %u\n"
	       "Inodes table block : %u\n"
	       "Free blocks count  : %u\n"
	       "Free inodes count  : %u\n"
	       "Directories count  : %u\n"
	       ,
	       group.bg_block_bitmap,
	       group.bg_inode_bitmap,
	       group.bg_inode_table,
	       group.bg_free_blocks_count,
	       group.bg_free_inodes_count,
	       group.bg_used_dirs_count);    /* directories count */
	//free(bitmap); // free() causes error


	exit(0);
}
