/*

Program to display the contents of used inodes on disk using the ext2 FS.






*/
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ext2fs/ext2_fs.h>

#define BASE_OFFSET 1024                   /* locates beginning of the super block (first group) */
#define FD_DEVICE "/dev/sdb1"               /* the floppy disk device */
#define BLOCK_OFFSET(block) ((block-1)*block_size)
static unsigned int block_size = 0;        /* block size (to be calculated) */

int main(void)
{
	struct ext2_super_block super;
	struct ext2_group_desc group;
	int fd;
	unsigned char *bitmap;
	/* open floppy device */

	if ((fd = open(FD_DEVICE, O_RDONLY)) < 0) {
		perror(FD_DEVICE);
		exit(1);  /* error while opening the floppy device */
	}

	/* read super-block */

	lseek(fd, BASE_OFFSET, SEEK_SET); 
	read(fd, &super, sizeof(super));

	if (super.s_magic != EXT2_SUPER_MAGIC) {
		fprintf(stderr, "Not a Ext2 filesystem\n");
		exit(1);
	} 
		
	block_size = 1024 << super.s_log_block_size;

	/* read group descriptor */

	lseek(fd,block_size, SEEK_SET);
	read(fd, &group, sizeof(group));

	bitmap = malloc(block_size);    /* allocate memory for the bitmap */
	lseek(fd, BLOCK_OFFSET(group.bg_block_bitmap), SEEK_SET);
	read(fd, bitmap, block_size);   /* read bitmap from disk */

	close(fd);
	
	printf("block bitmap %x\n",bitmap);
	
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
	free(bitmap);
	exit(0);
} /* main() */
