/*
This program displays the contents of used inodes on disk using the ext4 FS layout .
This code is written for this file "/home/saisha/Desktop/Ph.D/lustre/mds" in particular.

-------------------info from superblock-------------------------
total no. of blocks : 12,800
no. of blocks per group (according to layout): 32,768
hence, total no. of groups : 1
hence, total number of flex group : 1
total no. of inodes : 20480
one inode size : 1024 bytes
----------------------------------------------------------------
*/

/* lbraries required foe neo4j client*/


#include <neo4j-client.h>
#include <errno.h>
#include <time.h>

/*-----------------------------------*/

/* libraries for mds inode + attribute reading code*/
#include <math.h>
#include <inttypes.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "ext4_checker.h"
#include "xattr_sk.h"
/*------------------------------------------------------------------------------------------------------*/
#define FD_DEVICE "/home/saisha/Desktop/Ph.D/lustre/aged/mds" /*mounted file system*/
#define EXT4_SUPER_MAGIC 0xEF53								  /*The magic number signifies what kind of filesystem it is.*/
#define BASE_OFFSET 1024									  /* locates beginning of the super block (first group) */
#define BLOCK_OFFSET(block) ((block)*block_size)			  /* to be used for locations*/
/*------------------------------------------------------------------------------------------------------*/
static unsigned int block_size = 0; /* block size (to be calculated) */
/*------------------------------------------------------------------------------------------------------*/
#define ERANGE 34
#define ENODATA 51
#define EIO 5
#define EINVAL 22 /* Invalid argument */
/*------------------------------------------------------------------------------------------------------*/
#define IHDR(inode)                                                \
	((struct ext4_xattr_ibody_header *)((void *)inode +            \
										EXT4_GOOD_OLD_INODE_SIZE + \
										32))
/*------------------------------------------------------------------------------------------------------*/
#define IFIRST(hdr) ((struct ext4_xattr_entry *)((hdr) + 1))
/*------------------------------------------------------------------------------------------------------*/
#define IS_LAST_ENTRY(entry) (*(__u32 *)(entry) == 0)
#define EXT4_XATTR_NEXT(entry) \
	((struct ext4_xattr_entry *)((char *)(entry) + EXT4_XATTR_LEN((entry)->e_name_len)))
#define EXT4_XATTR_LEN(name_len)         \
	(((name_len) + EXT4_XATTR_ROUND +    \
	  sizeof(struct ext4_xattr_entry)) & \
	 ~EXT4_XATTR_ROUND)
#define EXT4_XATTR_PAD_BITS 2
#define EXT4_XATTR_PAD (1 << EXT4_XATTR_PAD_BITS)
#define EXT4_XATTR_ROUND (EXT4_XATTR_PAD - 1)
/*------------------------------------------------------------------------------------------------------*/

int main(void)
{

	//neo4j
	neo4j_client_init(); //NEO4j client initialization. 
	int neo_count =0;



	int lov =0;



	//mds
	int inode_size_value = 1024; //As per super-block info
	int fd;
	struct ext4_super_block super;
	struct ext4_group_desc group;
	unsigned char *bitmap = 0;
	unsigned char *ibitmap = 0;
	unsigned char *bitmap_fx = 0;
	unsigned char *ibitmap_fx = 0;
	struct inode test_inode;

	//using kernel declarations
	struct ext4_xattr_entry *first_entry;
	void *end;
	int name_index = 1;

	struct ext4_xattr_header test_extd;
	struct ext4_xattr_ibody_header test_iextd;
	struct ext4_xattr_entry test_ientry;
	struct ext4_xattr_entry test_ientry2;

	unsigned char *value2 = 0;
	unsigned char *value2_ext = 0;

	/* open local fs device */
	if ((fd = open(FD_DEVICE, O_RDONLY)) < 0)
	{
		perror(FD_DEVICE);
		exit(1); /* error while opening the floppy device */
	}

	////////////////////////////////////--SUPER BLOCK--//////////////////////////////////
	/* read super-block */
	lseek(fd, BASE_OFFSET, SEEK_SET);
	read(fd, &super, sizeof(super));

	//Magic number
	if (super.s_magic != EXT4_SUPER_MAGIC)
	{
		fprintf(stderr, "Not a Ext4 filesystem\n");
		exit(1);
	}

	//super.s_log_block_size is equal to 2
	//Hence, block_size equals 4096
	block_size = 1024 << super.s_log_block_size;

	printf("\nSuper block information:\n");
	printf("No.of reserved GDT blocks:%u\n"
		   "Total No. of blocks:%u\n"
		   "No. of free blocks:%u\n"
		   "No.of blocks per group:%u\n"
		   "Total No. of Inodes :%u\n"
		   "No. of free inodes: %u\n"
		   "No. of inodes per group:%u\n"
		   "Size of inode structure:%u\n"
		   "Block number of this superblock:%d\n"
		   "File system state:%u\n",
		   super.s_reserved_gdt_blocks,
		   super.s_blocks_count_lo,
		   super.s_free_blocks_count_lo,
		   super.s_blocks_per_group,
		   super.s_inodes_count,
		   super.s_free_inodes_count,
		   super.s_inodes_per_group,
		   super.s_inode_size,
		   super.s_block_group_nr,
		   super.s_state);
	//////////////////////////////////////////////////////////////////////////////////////////////

	///////////////////////////////////--GDT--/////////////////////////////////////////////////
	lseek(fd, BLOCK_OFFSET(1), SEEK_SET);
	read(fd, &group, sizeof(group));

	printf("\nReading first group-descriptor from device " FD_DEVICE ":\n"
		   "Inode Flag :%u\n"
		   "Block bitmap location: %u\n"
		   "Inode bitmap location: %u\n"
		   "Inode table location : %u\n"
		   "Free blocks count  : %u\n"
		   "Free inodes count  : %u\n"
		   "Directories count  : %u\n\n",
		   group.bg_flags,
		   group.bg_block_bitmap,
		   group.bg_inode_bitmap,
		   group.bg_inode_table,
		   group.bg_free_blocks_count,
		   group.bg_free_inodes_count,
		   group.bg_used_dirs_count);
	////////////////////////////////////////////////////////////////////////////////////////

	int group_number = 0; //this image has only one group.
	//AS there is only one group and one flex group

	/*---------------------------------- Flex Groups-------------------------------------
printf("Enter the group number from 0 to 2: \n");
scanf("%d",&group_number);*/

	////////////////////////////////--BLOCK_BITMAPS--////////////////////////////////////////////
	int block_bitmap_count = 0;

	//Blockbitmap for first group
	//printf("Block Bitmap for first group\n");
	uint64_t first_bitmap = 0;
	first_bitmap = group.bg_block_bitmap + group_number; // can add 1 to 3 as total 4 groups
	printf("\nBitmap block: %d\n", first_bitmap);
	//read "Block" bitmap
	bitmap = (unsigned char *)malloc(block_size); // allocate memory for the bitmap
	lseek(fd, BLOCK_OFFSET(first_bitmap), SEEK_SET);
	read(fd, bitmap, block_size); // read bitmap from disk
								  // Reading the Block Bitmap from the group descriptor
	int i, j;
	for (i = 0; i < 1600; i++) // 1600 x 8 = 12800 (i.e total nunmbe of blocks)
	{
		for (j = 0; j < 8; j++)
		{
			//printf("%d", *(bitmap)>>j & 1);
			if (*(bitmap) >> j & 1 == 1)
				block_bitmap_count++;
		}
		bitmap++;
		//printf(" \n");
	}

	printf("\nTotal number of used blocks:%d\n", block_bitmap_count);

	int total_block = 32768; //per group (according to layout) : info from superblock
							 //if (group_number == 3)
							 //total_block=6880;

	//printf("Total number of active blocks : %d of %d in this group\n", block_bitmap_count,total_block);
	/////////////////////////////////////////////////////////////////////////////////////////////////////

	//----------------------------------INODE_BITMAPS--////////////////////////////////////////////

	int inode_bitmap_count = 0;
	uint64_t first_ibitmap = 0;
	first_ibitmap = group.bg_inode_bitmap + group_number; // can add 1 to 3 as total 4 groups
	printf("\nInode bitmap: %d\n", first_ibitmap);

	ibitmap = (unsigned char *)malloc(block_size); //allocate memory for the bitmap
	lseek(fd, BLOCK_OFFSET(first_ibitmap), SEEK_SET);
	read(fd, ibitmap, block_size); //read bitmap from disk
								   // Reading the Inode Bitmap from the group descriptor
								   //printf("Inode bitmap:\n");
	//int i, j;
	for (i = 0; i < 2560; i++)  //20480  = 2560 x 8
	{ //2560
		for (j = 0; j < 8; j++)
		{
			//printf("%d", *(ibitmap)>>j & 1);
			if (*(ibitmap) >> j & 1 == 1)
				inode_bitmap_count++;
		}
		ibitmap++;
		// printf(" \n");
	}
	printf("Total number of active inodes : %d \n", inode_bitmap_count);
	/////////////////////////////////////////////////////////////////////////////////////////////////////



	/*----------------------------------------NEO4J CONNECTION------------------------------------------*/

	 /* use NEO4J_INSECURE when connecting to disable TLS */
	/*
    	neo4j_connection_t *connection =
            neo4j_connect("neo4j://neo4j:sk@localhost:11003", NULL, NEO4J_INSECURE);
    	if (connection == NULL)
    	{
        	neo4j_perror(stderr, errno, "Connection failed");
        	return EXIT_FAILURE;
    	}
	char buffer[100];
	*/
	/*--------------------------------------------------------------------------------------------------*/
	










	int int_count = 0; //count of internal attributes per inode.
	int ext_count = 0; //count of external attributes per inode.

	int attr_count = 0;	  //initializing attributes to be counted for each inode.
	int cur_ind = 0;	  //index to locate next attribute.
	int prev_ind = 0;	  //variable to sum all previous indices.
	int prev_ind_ext = 0; // for external block extended attribte.

	/////////////////////////////////////////--INODE--/////////////////////////////////////////////////////

	//inode_base_block: Depends on number of group
	int inode_base_block = group.bg_inode_table;
	//printf("Inode base block number is: %d\n", inode_base_block);

	ibitmap = (unsigned char *)malloc(block_size); //allocate memory for the bitmap
	lseek(fd, BLOCK_OFFSET(first_ibitmap), SEEK_SET);
	read(fd, ibitmap, block_size);
	for (i = 0; i < 2560; i++)
	{
		for (j = 0; j < 8; j++)
		{
			if (*(ibitmap) >> j & 1 == 1)

			{
				int inode_block_number = inode_base_block + (i);
				int inode_index = (j);

				uint64_t inode_location = BLOCK_OFFSET(inode_block_number) + ((inode_index) * (inode_size_value));
				lseek(fd, inode_location, SEEK_SET);
				read(fd, &test_inode, inode_size_value);

				//if(test_inode.i_size_lo!=0)
				//if(test_inode.i_mode!=0 || test_inode.i_flags !=0)
				if ((test_inode.i_mode != 0 || test_inode.i_flags != 0) && test_inode.i_mode != FILE_MODE)
				{

					printf("\n\n\n\n-------------------------Reading INODE-----------------------------\n");
					printf("inode block number %d\n", inode_location / 4096);
					printf("inode index: %d\n", inode_index);

					printf("i_extra_isize:%d\n", test_inode.i_extra_isize);
					printf("i_file_acl_lo:%d\n", test_inode.i_file_acl_lo);
					printf("l_i_file_acl_hi:%d\n", test_inode.osd2.linux2.l_i_file_acl_high);
					//Printing Inode Information
					//printf("Inode Information:\n");
					printf("Inode owner ID :%u\n"
						   "Inode blocks count :%u\n"
						   "File flags : %hu\n"
						   "File mode: %x\n",
						   test_inode.i_uid,
						   test_inode.i_blocks_lo,
						   test_inode.i_flags,
						   test_inode.i_mode);

					//-----till here we have got the inode-------//

					//getting header of the attributes.
					struct ext4_xattr_ibody_header *header;
					//header = IHDR(&test_inode, &raw_inode);
					header = IHDR(&test_inode);

					if (header->h_magic == ATTR_MAGIC)
					{
						printf("\nATTRIBUTE HEADER MATCHES \n");
						int_count++;
						//printf("AFTER THE HEADER\n");

						//getiing first attribute entry
						first_entry = IFIRST(header);
						//printing the name of the first attribute enrty(should be lma)
						printf("size of the first entry name length:%d\n", first_entry->e_name_len);
						printf("Name of the first entry is:");

						int name_itr = 0;
						for (name_itr = 0; name_itr <= first_entry->e_name_len; name_itr++)
						{

							printf("%c", first_entry->e_name[name_itr]);
						}
						printf("\n");

						//get the end of the attributes for the particular inode
						end = &test_inode + inode_size_value;

						/*------------------------------------------------------------------------------------
						read attribute enty names.
						start from first entry, then add name.length
						then next entry till we find an entry =0 ( i.e no more entries)
						------------------------------------------------------------------------------------*/

						printf("\n\n------------Inside the MAIN'S:  CHECK NAMES function------------------\n");
						printf("ENTRY names:\n ");
						//int lma_count;
						struct ext4_xattr_entry *e = first_entry;

						while (!IS_LAST_ENTRY(e))
						{

							//--------------------------------------------------
							int name_itr = 0;
							for (name_itr = 0; name_itr <= e->e_name_len; name_itr++)
							{

								printf("%c", e->e_name[name_itr]);
								if (e->e_name[2] == 'v')
								lov =1;
								


							}
							printf("\n");
							if (lov==1)
							{
							printf("\n");
							printf(" SK: lov exists\n");
							lov =0;}

							
							
							//----------------------------------------------------------------

							//SK: here we get names for each attribute.
							
	
							



							struct ext4_xattr_entry *next = EXT4_XATTR_NEXT(e);
							if ((void *)next >= end)
								return -EIO;
							e = next;
						}
						printf("---------------CHECK NAMES COMPLETED SUCCESSFULLY----------------------\n");

						//value for each entry
						void *value_start = first_entry;
						struct ext4_xattr_entry *entry = first_entry;

						while (!IS_LAST_ENTRY(entry))
						{
							if (entry->e_value_size != 0 &&
								entry->e_value_block == 0 &&
								(value_start + (entry->e_value_offs) <
									 (void *)e + sizeof(__u32) ||
								 value_start + (entry->e_value_offs) +
										 (entry->e_value_size) >
									 end))
								return -EIO;

							
								

							printf("Printing values_size :%d\n", entry->e_value_size);

							unsigned char value[entry->e_value_size + 1];
							memset(value, '\0', sizeof(char) * (entry->e_value_size + 1));
							lseek(fd, value_start + (entry->e_value_offs), SEEK_SET);
							read(fd, value, entry->e_value_size);

							printf("value: ");
							int z = 0;
							for (z = 0; z < entry->e_value_size; z++)
								printf("%x ", value[z]);
							printf("\n\n");

							
							// to identify only the lov attribute and further disect it.
								
								
								



														
							entry = EXT4_XATTR_NEXT(entry);



						}


						/*---------------------------------------------------------------------------------
						till here we have inode information,
						attribute names,
						attribute values.
						---------------------------------------------------------------------------------*/
						//char buffer[100];
						
							/*---------------------------neo4j code-------------------------------------*/
							/*
							sprintf(buffer,"CREATE (:SCANN {Inode_num: 1009});");
							neo_count++;
							neo4j_result_stream_t *results =
							neo4j_run(connection, buffer, neo4j_null);	
							neo4j_close_results(results);
							*/
							/*----------------------------------------------------------------*/

						

					} //closing braces for header confirmation part.
				}
			}
		}
		ibitmap++;
	}

	printf("\n\nint_count:%d\n", int_count);
	printf("\ninode_count= %d\n", inode_bitmap_count);

	



	/*--------------neo4j closing connection----------------*/
   	/*
	printf("\nneo_count= %d\n", neo_count);
	neo4j_close(connection);
    	neo4j_client_cleanup();
    	return EXIT_SUCCESS;
	*/
	/*------------------------------------------------------*/


	close(fd);

	exit(0);
}
