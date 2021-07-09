
#include <math.h>
#include <inttypes.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "ext4_checker.h"
#include "xattr_sk.h"

/* Neo4j: libraries required for neo4j client*/
#include <neo4j-client.h>
#include <errno.h>
#include <time.h>


#define FD_DEVICE "/home/saisha/Desktop/codes/aged/oss0" /*mounted file system*/
#define EXT4_SUPER_MAGIC 0xEF53								   /*The magic number signifies what kind of filesystem it is.*/
#define BASE_OFFSET 1024									   /* locates beginning of the super block (first group) */
#define BLOCK_OFFSET(block) ((block)*block_size)			   /* to be used for locations*/


/*xattr.c*/
#define IHDR(inode)                                                \
	((struct ext4_xattr_ibody_header *)((void *)inode +            \
										EXT4_GOOD_OLD_INODE_SIZE + \
										*inode.i_extra_isize   ))


#define IFIRST(hdr) ((struct ext4_xattr_entry *)((hdr) + 1))



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


	/*Neo4j: initializing neo4j-client*/
	neo4j_client_init();

	neo4j_connection_t *connection =
    neo4j_connect("neo4j://neo4j:sk@localhost:7687", NULL, NEO4J_INSECURE);
    //neo4j_connect("neo4j://neo4j@localhost:7687",NULL, NEO4J_INSECURE);
            
    if (connection == NULL)
    {
        neo4j_perror(stderr, errno, "Connection failed");
        return EXIT_FAILURE;
    }




    //block_size  = 2^(10 + super.s_log_block_size) : ext4 documentation
	static unsigned int block_size = 0;
	block_size = pow(2,10+super.s_log_block_size);

    /*reading group descriptor*/
	struct ext4_group_desc group;/*group descriptor*/
	lseek(fd, BLOCK_OFFSET(1), SEEK_SET);
	read(fd, &group, sizeof(group));


    struct ext4_inode cur_inode; /*dong: struct inode is an in-memory data structure; I change it to the on-disk inode structure*/
	
	int inode_table = group.bg_inode_table;/*get inode table location using group descriptor table*/
	
	/*get total number of inodes for this filesystem using info from super block*/
	int total_inodes = 0;
	total_inodes= super.s_inodes_count;
    
    /*get size of inode in bytes using info from super block*/
	int inode_size = 0;
	inode_size = super.s_inode_size;
	printf("inode sixze is %d bytes\n", super.s_inode_size);

	/*Neo4j: buffer*/
	char buffer[150];/*buffer for neo4j-client*/
	neo4j_result_stream_t *results;
	int xattr_count=0;

    for (int i = 1; i<= total_inodes; i++)
	{
		/*read the inode*/
        uint64_t inode_location = BLOCK_OFFSET(inode_table) + i*inode_size;		
		lseek(fd, inode_location, SEEK_SET);
		read(fd, &cur_inode, inode_size);

		//@todo: add code to check whether this inode is valid or not. 

		/*read xattr header*/
		struct ext4_xattr_ibody_header *header;
		header = IHDR(&cur_inode);

		/*check if the header has right magic number*/
		if (header->h_magic == ATTR_MAGIC)
		{
			printf("\n\n<-----------------------inode: %d------------------------------>\n", (i));
			//@dong: something needed to make sure this is a Lustre Inode
			xattr_count++;

			/*Neo4j: creating nodes for each inode*/
			sprintf(buffer, "CREATE (n:OSS {inode_number:%d});",(i));
			
			results = neo4j_run(connection, buffer, neo4j_null);
			neo4j_close_results(results);



			// determine whether this is a directory of normal file.
			if (S_ISDIR(cur_inode.i_mode) != 0) //S_IFDIR
			{
				printf("This is a directory: block_size: %u\n", block_size);

				/*Neo4j: type = Directory*/
				sprintf(buffer, "match(n:OSS) where n.inode_number=%d set n.inode_type='directory';",i);
				results = neo4j_run(connection, buffer, neo4j_null);
				neo4j_close_results(results);
				
				void *block_buf = (void *) malloc (block_size);
				int block_id = 0;
				int reach_the_end = 0;

				if (!reach_the_end && block_id < 12)
				{
					lseek(fd, cur_inode.i_block[block_id] * block_size, SEEK_SET);
					read(fd, block_buf, block_size);
					struct ext4_dir_entry_2 *ed;
					void *p = block_buf;

					while (p < block_buf + block_size)
					{
						ed = (struct ext4_dir_entry_2 *) p;
						if (ed->inode != 0)
							printf("inode: %d, rec_len: %hu, name: %s\n", ed->inode, ed->rec_len, ed->name);
						else {
							reach_the_end = 1;
							break;
						}
						p = p + ed->rec_len;
					}
					block_id += 1;
				}
				if (block_id >= 12){
					printf("ERROR, we encounter a huge directory and have not implemented the indirect block yet\n");
					exit(0);
				}
			}
			else if (S_ISREG(cur_inode.i_mode) != 0) //S_IFREG
			{
				printf("This is a file:\n");
				/*Neo4j: type = File*/
				sprintf(buffer, "match(n:OSS) where n.inode_number=%d set n.inode_type='file';",i);
				results = neo4j_run(connection, buffer, neo4j_null);
				neo4j_close_results(results);
			}

			/*pointer to the first xattr entry*/
			struct ext4_xattr_entry *first_entry;
			first_entry = IFIRST(header);

			/*end of the inode*/
			void *end;
			end = &cur_inode + inode_size;


			/*"entry" is used to iterate through all the xattr entries*/
			struct ext4_xattr_entry *entry = first_entry;

            		/*iterate through each xattr entry of the inode*/
			while (!IS_LAST_ENTRY(entry))
			{

				/*printing the entry name*/
				for (int name_itr = 0; name_itr < entry->e_name_len; name_itr++)
				{
					
					printf("%c", entry->e_name[name_itr]);
					
				}
				//printf("\n");
				

				



                /*needs to be changed into a macro*/
                uint64_t value_location = inode_location +EXT4_GOOD_OLD_INODE_SIZE +cur_inode.i_extra_isize + sizeof(struct ext4_xattr_ibody_header) +entry->e_value_offs;
				

				/*identify if it is lma xattr: check if the entry name is "lma"*/
				if((char)entry->e_name[0] == 'l' && (char)entry->e_name[1] =='m' && (char)entry->e_name[2] == 'a')
				{
					
					// printf("\nThis is info about LMA of OST\n");
					struct lustre_mdt_attrs lma; /*fid from lma*/
					lseek(fd, value_location, SEEK_SET);
					read(fd, &lma, sizeof(struct lustre_mdt_attrs));
					//struct lustre_mdt_attrs * lma = (struct lustre_mdt_attrs *) value;
					printf(": [0x%llx:0x%x:0x%x]\n", lma.lma_self_fid.f_seq, lma.lma_self_fid.f_oid, lma.lma_self_fid.f_ver);

					/*Neo4j: lma*/
					sprintf(buffer, "match(n:OSS) where n.inode_number=%d set n.lma='0x%llx:0x%x:0x%x';",i,lma.lma_self_fid.f_seq, lma.lma_self_fid.f_oid, lma.lma_self_fid.f_ver);
					results = neo4j_run(connection, buffer, neo4j_null);
					neo4j_close_results(results);

					sprintf(buffer, "match(n:OSS) where n.inode_number=%d set n.f_oid='%x';",i,lma.lma_self_fid.f_oid);
					results = neo4j_run(connection, buffer, neo4j_null);
					neo4j_close_results(results);

				}







				/*identify if it is fid xattr: check if the entry name is "fid"*/
				if((char)entry->e_name[0] == 'f' && (char)entry->e_name[1] =='i' && (char)entry->e_name[2] == 'd')
				{

					//struct filter_fid_18_23 fid; /*parent fid of the OST*/
					struct filter_fid fid; /*parent fid of the OST*/
					lseek(fd, value_location, SEEK_SET);
					read(fd, &fid, sizeof(struct filter_fid));

					printf(": [0x%llx:0x%x:0x%x]\n", fid.ff_parent.f_seq, fid.ff_parent.f_oid, fid.ff_parent.f_ver);
					printf("stripe size: %u bytes, stripe count: %u\n", fid.ff_layout.ol_stripe_size, fid.ff_layout.ol_stripe_count);
					printf("ff_layout_version:%u, ff_range:%u\n",fid.ff_layout_version, fid.ff_range);
					printf("comp_start:%llx, comp_end:%llx, component id:%u\n",fid.ff_layout.ol_comp_start, fid.ff_layout.ol_comp_end  ,fid.ff_layout.ol_comp_id);
					//printf("ff_objid: %llu, ff_seq: %llu\n", fid.ff_objid, fid.ff_seq);

					/*Neo4j: fid*/
					sprintf(buffer, "match(n:OSS) where n.inode_number=%d set n.fid='0x%llx:0x%x:0x%x';",i,fid.ff_parent.f_seq, fid.ff_parent.f_oid, fid.ff_parent.f_ver);
					results = neo4j_run(connection, buffer, neo4j_null);
					neo4j_close_results(results);
					/*Neo4j: relationship-child oss*/
					sprintf(buffer, "match (a:OSS),(b:MDS) where a.fid = b.lma merge (a)-[:OST_child_of]->(b);");
					results = neo4j_run(connection, buffer, neo4j_null);
					neo4j_close_results(results);
					


				}
				




                /*iterate to the next entry*/
				entry = EXT4_XATTR_NEXT(entry);

            }
        } 
 	}

    
	printf("inode count : %d\n", total_inodes);
	printf("xattr count : %d\n", xattr_count);

	/*This is the part of MDS.*/
	/*Neo4j: relationship-parent mds*/
	int mds_total_inodes = 700; /*will have to change this number to total number of inodes on MDS.*/
	for (int j = 1; j <= mds_total_inodes; j++)
	{
		/*create the LOV_parent relationship from MDS to OSS when MDS.Lov = OSS.LMA*/
	sprintf(buffer, "match(n:MDS),(m:OSS) where n.inode_number=%d and n.ost_oid = m.f_oid merge (n)-[:LOV_parent_of]->(m);",j);
	neo4j_result_stream_t *results =
	neo4j_run(connection, buffer, neo4j_null);
	neo4j_close_results(results);
	}





	close(fd);




	exit(0);
}
