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








// SK: April 2021
//Lov-EA 




// other interdependent required structs

struct lu_extent {
       __u64 e_start;
       __u64 e_end;
};






struct lu_fid {
       /**
       * FID sequence. Sequence is a unit of migration: all files (objects)
        * with FIDs from a given sequence are stored on the same server.
        * Lustre should support 2^64 objects, so even if each sequence
        * has only a single object we can still enumerate 2^64 objects.
        **/
         __u64 f_seq;
         /* FID number within sequence. */
        __u32 f_oid;
         /**
         * FID version, used to distinguish different versions (in the sense
         * of snapshots, etc.) of the same file system object. Not currently
         * used.
         **/
        __u32 f_ver;
 };



struct ost_id {
        union {
                struct {
                        __u64   oi_id;
                        __u64   oi_seq;
                } oi;
                struct lu_fid oi_fid;
         };
 };



struct lov_ost_data_v1 {          /* per-stripe data structure (little-endian)*/
        struct ost_id l_ost_oi;   /* OST object ID */
        __u32 l_ost_gen;          /* generation of this l_ost_idx */
        __u32 l_ost_idx;          /* OST index in LOV (lov_tgt_desc->tgts) */
};








#define 	LOV_MAGIC_V1   0x0BD10BD0
//This is struct for layout header 
struct lov_mds_md_v1 {            /* LOV EA mds/wire data (little-endian) */
        __u32 lmm_magic;          /* magic number = LOV_MAGIC_V1 */
        __u32 lmm_pattern;        /* LOV_PATTERN_RAID0, LOV_PATTERN_RAID1 */
        //struct ost_id lmm_oi;   /* LOV object ID */
	struct ost_id;   
     __u32 lmm_stripe_size;    /* size of stripe in bytes */
        /* lmm_stripe_count used to be __u32 */
        __u16 lmm_stripe_count;   /* num stripes in use for this object */
        __u16 lmm_layout_gen;     /* layout generation number */
        struct lov_ost_data_v1 lmm_objects[0]; /* per-stripe data */
};




//This struct is for composite entry
struct lov_comp_md_entry_v1 {
        __u32 lcme_id;                  /* unique identifier of component */
        __u32 lcme_flags;               /* LCME_FL_XXX */
        struct lu_extent lcme_extent;   /* file extent for component */
        __u32 lcme_offset;              /* offset of component blob in layout */
        __u32 lcme_size;                /* size of component blob data */
        __u64 lcme_padding[2];
};



LOV_MAGIC_COMP_V1 = 0x0BD40BD0;
//This struct is for composite header
struct lov_comp_md_v1 {
        __u32 lcm_magic;        /* LOV_MAGIC_COMP_V1 */
        __u32 lcm_size;         /* overall size of layout including this structure */
        __u32 lcm_layout_gen;
        __u16 lcm_flags;
        __u16 lcm_entry_count;
        __u64 lcm_padding1;
        __u64 lcm_padding2;
        struct lov_comp_md_entry_v1 lcm_entries[0];
};














