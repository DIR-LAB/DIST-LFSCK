/*
  Program to display the contents of used inodes on disk using the ext4 FS.
*/

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

#define FD_DEVICE "/dev/loop16"    /*mounted file system*/
#define EXT4_SUPER_MAGIC 0xEF53  /*The magic number signifies what kind of filesystem it is.*/
#define BASE_OFFSET 1024         /* locates beginning of the super block (first group) */
#define BLOCK_OFFSET(block) ((block)*block_size) /* to be used for locations*/

static unsigned int block_size = 0;        /* block size (to be calculated) */

int main(void)
{
  int fd;
  struct ext4_super_block super;
  struct ext4_group_desc group;
  unsigned char *bitmap=0;
  unsigned char *ibitmap=0;
  struct ext4_inode test_inode;

  struct ext4_xattr_header test_extd;
  struct ext4_xattr_ibody_header test_iextd;
  struct ext4_xattr_entry test_ientry;
  //struct ext4_xattr_entry test_ientry2;

  unsigned char *value2=0;
  unsigned char *value2_ext=0;

  /* open local fs device */
  if ((fd = open(FD_DEVICE, O_RDONLY)) < 0) {
    perror(FD_DEVICE);
    exit(1);  /* error while opening the floppy device */
  }


  /* ------------- SUPER BLOCK Start ------------- */

  /* read super-block */
  lseek(fd, BASE_OFFSET , SEEK_SET);
  read(fd, &super, sizeof(super));


  //Magic number
  if (super.s_magic != EXT4_SUPER_MAGIC) {
    fprintf(stderr, "Not a Ext4 filesystem\n");
    exit(1);
  }

  //super.s_log_block_size is equal to 2
  //Hence, block_size equals 4096
  block_size = 1024 << super.s_log_block_size;

  //This give number of groups per flex_bg
  int flex_bg = (int)pow(2,super.s_log_groups_per_flex); //16 for this fs
  printf("\n flex group size: %d\n", flex_bg);

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
  /* ------------- SUPER BLOCK End  ------------- */


  /* ------------- GDT Start ------------- */
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
  /* ------------- GDT End ------------- */


  /* ------------- Flex  ------------- */
  //This fs has 1 flex_group. @dong: can this file handles more than 1 flex_group?
  int flex_grp_no = 0;
  if(flex_grp_no>0)
    {printf("ERROR: exceeded amount of flex_grp\n");
      exit(0);
    }

  //flex_bg = 16
  //flex_number = 16 * 32768 = 524288
  int flex_number = flex_bg * super.s_blocks_per_group;
  printf("\nFLEX NUMBER = %d\n",flex_number);
  /* ------------- Flex  ------------- */


  /* ------------- BLOCK_BITMAPS  ------------- */
  uint64_t first_bitmap = 0;
  uint64_t last_bitmap = 0;


  switch(flex_grp_no)
    {
	case 0:
      {
        first_bitmap = group.bg_block_bitmap;
        last_bitmap = (first_bitmap +flex_bg)-1;
        printf("\nFirst Bitmap block: %d\n", first_bitmap);
        printf("Last Bitmap block: %d\n", last_bitmap);
        printf("\nFirst Bitmap block address: %d\n", BLOCK_OFFSET(first_bitmap));
        for(int a = first_bitmap ; a<=last_bitmap ; a++)
          {
            //read "Block" bitmap
            bitmap = (unsigned char *)malloc(block_size);    // allocate memory for the bitmap
            lseek(fd, BLOCK_OFFSET(a), SEEK_SET);
            read(fd, bitmap, block_size);   // read bitmap from disk
            // Reading the Block Bitmap from the group descriptor
            printf("Block bitmap:\n");
            int i, j;
            for (i = 0; i < 10; i++) {
              for (j = 0; j < 8; j++) {
                printf("%d", *(bitmap)>>j & 1);
                //printf("%u\n",&(bitmap));
              }
              bitmap++;
              printf(" \n");
            }
            // BLOCK_OFFSET(group.bg_block_bitmap)
            //BLOCK_OFFSET(708)
          }
        break;
      }
	default :
      {
        printf("default block bitmap\n");
        break;
      }
    }
  /* ------------- BLOCK_BITMAPS  ------------- */

  /* ------------- INODE_BITMAPS  ------------- */
  uint64_t first_ibitmap = 0;
  uint64_t last_ibitmap = 0;


  switch(flex_grp_no)
    {
	case 0:
      {

        first_ibitmap = group.bg_inode_bitmap;
        last_ibitmap = (first_ibitmap +flex_bg)-1;

        printf("\nFirst Bitmap block: %d\n", first_ibitmap);
        printf("Last Bitmap block: %d\n", last_ibitmap);

        for(int a = first_ibitmap ; a<=last_ibitmap ; a++)
          {
            //read "Inode" bitmap
            ibitmap = (unsigned char *)malloc(block_size);    //allocate memory for the bitmap
            lseek(fd, BLOCK_OFFSET(a), SEEK_SET);
            read(fd, ibitmap, block_size);   //read bitmap from disk
            // Reading the Inode Bitmap from the group descriptor
            printf("Inode bitmap:\n");
            int i, j;
            for (i = 0; i < 10; i++) {
              for (j = 0; j < 8; j++) {
                printf("%d", *(ibitmap)>>j & 1);
              }
              ibitmap++;
              printf(" \n");
            }
            //BLOCK_OFFSET(group.bg_inode_bitmap)
          }

        break;
      }
	default :
      {
        printf("default inode bitmap\n");
        break;
      }

    }
  /* ------------- INODE_BITMAPS  ------------- */



  int inode_count=0; //initializing inodes to be counted.
  int attr_count =0; //initializing attributes to be counted for each inode.
  int cur_ind=0; //index to locate next attribute.
  int prev_ind=0; //variable to sum all previous indices.
  int prev_ind_ext=0; // for external block extended attribte.

  /* ------------- INODE  ------------- */

  //Considering 8192 inodes in 512 blocks
  // 1 inode is made of 256 bytes. Hence using inode indexing

  //inode index can go upto 1 to 131072 for a particular flex_group.
  int inode_index = 1;
  int ext_count=0; //count of external attributes per inode.
  int int_count=0; //count of internal attributes per inode.

  switch(flex_grp_no)
    {
	case 0:
      {
        for(inode_index=1;inode_index<=32768;inode_index++)
          {
            uint64_t inode_location = BLOCK_OFFSET(group.bg_inode_table)+((inode_index-1)*(256));
            lseek(fd,inode_location,SEEK_SET);
            read(fd,&test_inode,256);
            if(test_inode.i_size_lo!=0)
              {
                inode_count++;
                printf("i_extra_isize:%d\n", test_inode.i_extra_isize);
                printf("i_file_acl_lo:%d\n", test_inode.i_file_acl_lo);
                printf("l_i_file_acl_hi:%d\n", test_inode.osd2.linux2.l_i_file_acl_high);
                printf("inode block number %d\n", inode_location/4096);
                printf("inode index: %d\n",inode_index);
                //Printing Inode Information
                printf("Inode Information:\n");
                printf("Inode Size in bytes:%u\n"
                       "Inode owner ID :%u\n"
                       "Inode blocks count :%u\n"
                       "File flags : %hu\n"
                       "File mode: %x\n\n",
                       test_inode.i_size_lo,
                       test_inode.i_uid,
                       test_inode.i_blocks_lo,
                       test_inode.i_flags,
                       test_inode.i_mode);
              }

            /* ------------- internal extended attributes  ------------- */
            uint64_t attr_head_location = BLOCK_OFFSET(group.bg_inode_table)+((inode_index-1)*(256)+(128+test_inode.i_extra_isize));
            lseek(fd,attr_head_location,SEEK_SET);
            read(fd,&test_iextd,4);
            if (test_iextd.h_magic == ATTR_MAGIC)
              {
                //printf("internal attribue\n");
                int_count++;
                printf("1st internal attribute:\n");
                printf("Extd Header Magic Number:%X\n",test_iextd.h_magic);
                printf("inode_index=%d\n", inode_index);
                //attribute entry
                uint64_t attr_location = attr_head_location+sizeof(test_iextd);
                lseek(fd,attr_location,SEEK_SET);
                read(fd,&test_ientry,sizeof(test_ientry));
                printf("xattr_entry:e_name_len:%d\n",test_ientry.e_name_len);
                printf("xattr_entrye_name_index:%d\n",test_ientry.e_name_index);
                printf("xattr_entry:e_value_offs:%d\n",test_ientry.e_value_offs);
                printf("xattr_entry:e_value_block:%d\n",test_ientry.e_value_block);
                printf("xattr_entry:e_value_size:%d\n",test_ientry.e_value_size);
                printf("xattr_entry:e_hash:%d\n",test_ientry.e_hash);

                //attribute value
                //entry provides offset relative to the first entry on in-inode attribute
                char value[test_ientry.e_value_size+1];
                memset( value, '\0', sizeof(char)*(test_ientry.e_value_size+1));
                lseek(fd,attr_location+test_ientry.e_value_offs,SEEK_SET);
                read(fd,value,test_ientry.e_value_size);
                printf("value:%s \n\n",value);

                for(attr_count=1;attr_count<3;attr_count++)
                  {

                    uint64_t attr2_location = attr_location+sizeof(test_ientry)*attr_count+prev_ind;
                    cur_ind = round((test_ientry.e_name_len-1)/4)*4+4;
                    prev_ind+=cur_ind;

                    lseek(fd,attr2_location+cur_ind,SEEK_SET);
                    read(fd,&test_ientry,sizeof(test_ientry));

                    char value2 [test_ientry.e_value_size+1];
                    memset( value2, '\0', sizeof(char)*(test_ientry.e_value_size+1));
                    lseek(fd,(attr_location+test_ientry.e_value_offs),SEEK_SET);
                    read(fd,value2,test_ientry.e_value_size);

                    if(test_ientry.e_value_size != 0)
                      { int_count++;
                        printf("Attribute no. : %d \n",attr_count+1);
                        printf("no. of bytes after prev attribute:%d\n",cur_ind);
                        printf("xattr_entry:e_name_len:%d\n",test_ientry.e_name_len);
                        printf("xattr_entrye_name_index:%d\n",test_ientry.e_name_index);
                        printf("xattr_entry:e_value_offs:%d\n",test_ientry.e_value_offs);
                        printf("xattr_entry:e_value_block:%d\n",test_ientry.e_value_block);
                        printf("xattr_entry:e_value_size:%d\n",test_ientry.e_value_size);
                        printf("xattr_entry:e_hash:%d\n",test_ientry.e_hash);
                        printf("value:%s \n\n",value2);
                      }
                  }
              }
            /* ------------- internal extended attributes  ------------- */

            /* ------------- external extended attribtues  ------------- */
            lseek(fd,BLOCK_OFFSET(test_inode.i_file_acl_lo),SEEK_SET);
            read(fd,&test_extd,sizeof(test_extd));
            if (test_extd.h_magic == ATTR_MAGIC)
              {
                printf("external attribue\n");
                ext_count++;
                printf("Extd Header Magic Number:%X\n",test_iextd.h_magic);
                printf("inode_index=%d\n", inode_index);
                printf("Ref Count:%d\n",test_extd.h_blocks);
                printf("Number of disk blocks used:%d\n",test_extd.h_refcount);
                printf("hash value of all attributes:%X\n",test_extd.h_hash);

                //get the entry for the inodes:
                lseek(fd,BLOCK_OFFSET(test_inode.i_file_acl_lo)+sizeof(test_extd),SEEK_SET);
                read(fd,&test_ientry,sizeof(test_ientry));
                printf("xattr_entry:e_name_len:%d\n",test_ientry.e_name_len);
                printf("xattr_entrye_name_index:%d\n",test_ientry.e_name_index);
                printf("xattr_entry:e_value_offs:%d\n",test_ientry.e_value_offs);
                printf("xattr_entry:e_value_block:%d\n",test_ientry.e_value_block);
                printf("xattr_entry:e_value_size:%d\n",test_ientry.e_value_size);
                printf("xattr_entry:e_hash:%d\n",test_ientry.e_hash);

                //extended attribute value
                //entry provides offset relative to the header
                char value_ext[test_ientry.e_value_size+1];
                memset( value_ext, '\0', sizeof(char)*(test_ientry.e_value_size+1));
                lseek(fd,BLOCK_OFFSET(test_inode.i_file_acl_lo)+test_ientry.e_value_offs,SEEK_SET);
                read(fd,value_ext,test_ientry.e_value_size);
                printf("value:%s \n\n",value_ext);


                for(attr_count=1;attr_count<10;attr_count++)
                  {
                    uint64_t attr2_location_ext = BLOCK_OFFSET(test_inode.i_file_acl_lo)+sizeof(test_extd)+sizeof(test_ientry)*attr_count+prev_ind_ext;
                    cur_ind = round((test_ientry.e_name_len-1)/4)*4+4;
                    prev_ind_ext+=cur_ind;

                    lseek(fd,attr2_location_ext+cur_ind,SEEK_SET);
                    read(fd,&test_ientry,sizeof(test_ientry));

                    char value[test_ientry.e_value_size+1];
                    memset( value, '\0', sizeof(char)*(test_ientry.e_value_size+1));
                    lseek(fd,BLOCK_OFFSET(test_inode.i_file_acl_lo)+test_ientry.e_value_offs,SEEK_SET);
                    read(fd,value,test_ientry.e_value_size);

                    if(test_ientry.e_value_size != 0)
                      {
                        ext_count++;
                        printf("External Attribute no. : %d \n",attr_count+1);
                        printf("no. of bytes after prev attribute:%d\n",cur_ind);
                        printf("xattr_entry:e_name_len:%d\n",test_ientry.e_name_len);
                        printf("xattr_entrye_name_index:%d\n",test_ientry.e_name_index);
                        printf("xattr_entry:e_value_offs:%d\n",test_ientry.e_value_offs);
                        printf("xattr_entry:e_value_block:%d\n",test_ientry.e_value_block);
                        printf("xattr_entry:e_value_size:%d\n",test_ientry.e_value_size);
                        printf("xattr_entry:e_hash:%d\n",test_ientry.e_hash);
                        printf("value:%s\n\n",value);
                      }
                  }
              }
            ///////////////////////////////////////////////////////////////////////////////////////////////////////////
          }
        printf("int_count:%d\n",int_count);
        printf("ext_count:%d\n\n",ext_count);
        break;
      }
	default :
      {
        printf("default inode tables\n");
        break;
      }
    }

  printf("inode_count=%d\n",inode_count);

  close(fd);

  exit(0);
}
