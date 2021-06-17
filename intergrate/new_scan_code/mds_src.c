


/* libraries for mds inode + attribute reading code*/
#include <math.h>
#include <inttypes.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
//#include "endian.h"
#include "ext4_checker.h"
#include "xattr_sk.h"

/* libraries required foe neo4j client*/
//#include <neo4j-client.h>
#include <errno.h>
#include <time.h>


#define FD_DEVICE "/home/saisha/Desktop/codes/aged/mds" /*mounted file system*/

/*magic.h*/
#define EXT4_SUPER_MAGIC 0xEF53	/*The magic number signifies what kind of filesystem it is.*/

/*super block starts 1024 bytes from the begining of the disk*/
#define BASE_OFFSET 1024 /* locates beginning of the super block (first group) */
#define BLOCK_OFFSET(block) ((block)*block_size) /* to be used to retrieve locations*/



/*xattr.c*/
#define IHDR(inode)                                                \
	((struct ext4_xattr_ibody_header *)((void *)inode +            \
										EXT4_GOOD_OLD_INODE_SIZE + \
										*inode.i_extra_isize   ))

#define IFIRST(hdr) ((struct ext4_xattr_entry *)((hdr) + 1))

/*sk:value location MACRO for an xattr*/
/* needs more work
#define VALUE_LOC(inode,ext4_xattr_entry) (uint64_t)((void *)inode +            \
										EXT4_GOOD_OLD_INODE_SIZE + \
										32 + sizeof(struct ext4_xattr_ibody_header) + ext4_xattr_entry->e_value_offs)
										*/

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






int main(void)
{

	/*file to note the entries and values*/
	FILE * fPtr;
	fPtr = fopen("file1.txt", "w");
	if(fPtr == NULL)
	{
        /* File not created hence exit */
        printf("Unable to create file.\n");
        exit(EXIT_FAILURE);
	}



	/*initializing neo4j-client*/
	//neo4j_client_init(); 
	

	/*accessing the disk*/
	int fd;

	/* open fs device */
	if ((fd = open(FD_DEVICE, O_RDONLY)) < 0)
	{
		perror(FD_DEVICE);
		exit(1); /* error while opening the device */
	}


	/* read super-block */
	struct ext4_super_block super;/*superblock*/
	lseek(fd, BASE_OFFSET, SEEK_SET);
	read(fd, &super, sizeof(super));

	/*check to match ext4 Magic number*/
	if (super.s_magic != EXT4_SUPER_MAGIC)
	{
		fprintf(stderr, "Not a Ext4 filesystem\n");
		exit(1);
	}

	//block_size  = 2^(10 + super.s_log_block_size) : ext4 documentation
	static unsigned int block_size = 0;
	block_size = pow(2,10+super.s_log_block_size);
	

	/*reading group descriptor*/
	struct ext4_group_desc group;/*group descriptor*/
	lseek(fd, BLOCK_OFFSET(1), SEEK_SET);
	read(fd, &group, sizeof(group));


	struct inode test_inode; /*ext4 inode struct*/
	int inode_table = group.bg_inode_table;/*get inode table location using group descriptor table*/
	
	/*get total number if inodes for this filesystem using info from super block*/
	int total_inodes = 0;
	total_inodes= super.s_inodes_count;
	

	/*get size of inode in bytes using info from super block*/
	int inode_size = 0;
	inode_size = super.s_inode_size;
	

	int entry_count=0; /*used to check total number of entries*/
	for (int i = 0; i< total_inodes; i++)
	{
		uint64_t inode_location = BLOCK_OFFSET(inode_table) + i*inode_size;
				
		lseek(fd, inode_location, SEEK_SET);
		read(fd, &test_inode, inode_size);

								
		/*read xattr header*/
		struct ext4_xattr_ibody_header *header;
		header = IHDR(&test_inode);

		/*check if the header has right magic number*/
		if (header->h_magic == ATTR_MAGIC)
		{
			/*used to keep check on number if total inodes having xattr header magic number match*/			
			entry_count ++;

			/*pointer to the first xattr entry*/
			struct ext4_xattr_entry *first_entry;
			first_entry = IFIRST(header);

			/*end of the inode*/
			void *end;
			end = &test_inode + inode_size;


			/*"entry" is used to iterate through all the xattr entries*/
			struct ext4_xattr_entry *entry = first_entry;


			while (!IS_LAST_ENTRY(entry))
			{
		
				/*printing the entry name*/
				for (int name_itr = 0; name_itr < entry->e_name_len; name_itr++)
				{
					
					printf("%c", entry->e_name[name_itr]);
					fprintf(fPtr,"%c",entry->e_name[name_itr]);
				}
				printf("\n");
				fprintf(fPtr,"\n");
				//fprintf(fPtr,"The value size: %d\n", entry->e_value_size);	/*printing size of value in bytes*/


				/*obtain and print the lov xattr value*/
				uint64_t value_location = inode_location +EXT4_GOOD_OLD_INODE_SIZE + test_inode.i_extra_isize+ sizeof(struct ext4_xattr_ibody_header) +entry->e_value_offs;
				


				/*print the xattr value in terminal and file1.txt*/
				unsigned char value[entry->e_value_size + 1];
				memset(value, '\0', sizeof(char) * (entry->e_value_size + 1));
				lseek(fd, value_location, SEEK_SET);
				read(fd, value, entry->e_value_size);
				printf("value: ");
				int z = 0;
				for (z = 0; z < entry->e_value_size; z++)
				{
				printf("%0.2x ", value[z]);
				fprintf(fPtr, "%0.2x",value[z]);
				}
				printf("\n\n");
				fprintf(fPtr, "\n\n");
				



				/*identify if it is lov xattr: check if the entry name is "lov"*/
				if((char)entry->e_name[0] == 'l' && (char)entry->e_name[1] =='o' && (char)entry->e_name[2] == 'v')
				{
					struct lov_user_md lum;
					lseek(fd, value_location, SEEK_SET);
					read(fd, &lum, sizeof(struct lov_user_md));
					
					
					
					
					 
					 
					 
					 int ent_count =0;
					if (lum.lmm_magic == LOV_MAGIC_COMP_V1) /*multple entries*/
					{
						//printf("%x",lum.lmm_magic);
						 printf("This lov has multiple compenent entries");
						 fprintf(fPtr,"It has multiple compenent entries");
						//comp_v1 = (struct lov_comp_md_v1 *)lum;
						//ent_count = comp_v1->lcm_entry_count;
						
					}
					else if (lum.lmm_magic == LOV_MAGIC_V1 || lum.lmm_magic == LOV_MAGIC_V3) /*single entry*/
					{
						// printf("%x\n",lum.lmm_magic);
						// printf("This lov have a single compenent entry\n");
						fprintf(fPtr,"It have a single compenent entry\n");
						ent_count = 1;	
					
					}

					
					
						/*printing wrt lum: user_md*/
						fprintf(fPtr,"stripe size : %d bytes\n",lum.lmm_stripe_size);
						fprintf(fPtr,"stripe count: %d number of stripes\n\n", lum.lmm_stripe_count);
						
						/*information about the MDT*/
						fprintf(fPtr,"Information about MDT\n");
						fprintf(fPtr,"oid:%0.2x\n", lum.lmm_oi.oi.oi_id);
						fprintf(fPtr,"o_seq:%0.2x\n", lum.lmm_oi.oi.oi_seq);
						fprintf(fPtr,"fid_seq : %0.2x\n", lum.lmm_oi.oi_fid.f_seq);
						fprintf(fPtr,"fid_id : %0.2x\n", lum.lmm_oi.oi_fid.f_oid);
						fprintf(fPtr,"fid_ver : %0.2x\n", lum.lmm_oi.oi_fid.f_ver);

						// /*information about the OST: per-stripe data*/
						fprintf(fPtr,"\n\nInformation about OST\n");
						int l =0;
						fprintf(fPtr,"l_ost_idx %x\n", lum.lmm_objects[l].l_ost_idx); /* OST index in LOV */
						fprintf(fPtr,"idx_gen: %x\n", lum.lmm_objects[l].l_ost_gen); /*Generation of this OST index */

						
						fprintf(fPtr,"oi.oi_id: %x\n", lum.lmm_objects[l].l_ost_oi.oi.oi_id); 
						fprintf(fPtr,"oi.oi_seq: %x\n", lum.lmm_objects[l].l_ost_oi.oi.oi_seq);



						fprintf(fPtr,"oi_fid.f_oid: %x\n", lum.lmm_objects[l].l_ost_oi.oi_fid.f_oid);
						fprintf(fPtr,"oi_fid.f_seq: %x\n", lum.lmm_objects[l].l_ost_oi.oi_fid.f_seq);
						fprintf(fPtr,"oi_fid.f_ver: %x\n", lum.lmm_objects[l].l_ost_oi.oi_fid.f_ver);
						/*---------------------------------*/

						
	



				}

				

				/*iterate to the next entry*/
				entry = EXT4_XATTR_NEXT(entry);



			}

		}
	}

	printf("entry_count: %d\n", entry_count);
	printf("inode count : %d\n", total_inodes);
	//printf("inode_bitmap_count: %d", inode_bitmap_count);
	//printf("size of header struct: %d\n",EXT4_GOOD_OLD_INODE_SIZE);

	fclose(fPtr);
	close(fd);

	exit(0);
}





