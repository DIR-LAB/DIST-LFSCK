//contains struct for internal and external attributes

#define ATTR_MAGIC 0xEA020000 /*magic number to verify extended attributes*/

//external
struct ext4_xattr_header
{
	__le32 h_magic;		 /* magic number for identification */
	__le32 h_refcount;	 /* reference count */
	__le32 h_blocks;	 /* number of disk blocks used */
	__le32 h_hash;		 /* hash value of all attributes */
	__le32 h_checksum;	 /* crc32c(uuid+id+xattrblock) */
						 /* id = inum if refcount=1, blknum otherwise */
	__u32 h_reserved[3]; /* zero right now */
};

//internal + external
struct ext4_xattr_ibody_header
{
	__le32 h_magic; /* magic number for identification */
};

//internal + external
struct ext4_xattr_entry
{
	__u8 e_name_len;	  /* length of name */
	__u8 e_name_index;	  /* attribute name index */
	__le16 e_value_offs;  /* offset in disk block of value */
	__le32 e_value_block; /* disk block attribute is stored on (n/i) */
	__le32 e_value_size;  /* size of attribute value */
	__le32 e_hash;		  /* hash value of name and value */
	char e_name[0];		  /* attribute name */
};
