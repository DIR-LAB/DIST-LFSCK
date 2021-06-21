#include <stdbool.h>

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

/*all the magic numbers related to lov xattr*/
#define LOV_MAGIC_MAGIC 0x0BD0
#define LOV_MAGIC_V1 (0x0BD10000 | LOV_MAGIC_MAGIC)
#define LOV_MAGIC_V3 (0x0BD30000 | LOV_MAGIC_MAGIC)
#define LOV_USER_MAGIC_V1 0x0BD10BD0
#define LOV_USER_MAGIC LOV_USER_MAGIC_V1
#define LOV_MAGIC_COMP_V1 (0x0BD60000 | LOV_MAGIC_MAGIC)

/*--------------------------*/
#define LOV_PATTERN_NONE 0x000
#define LOV_PATTERN_RAID0 0x001
#define LOV_PATTERN_RAID1 0x002
#define LOV_PATTERN_MDT 0x100
#define LOV_PATTERN_CMOBD 0x200

// other interdependent required structs

struct lu_extent
{
	__u64 e_start;
	__u64 e_end;
};

struct lu_fid
{
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

struct ost_id
{
	union
	{
		struct
		{
			__u64 oi_id;
			__u64 oi_seq;
		} oi;
		struct lu_fid oi_fid;
	};
};

struct lustre_mdt_attrs {
	/**
	 * Bitfield for supported data in this structure. From enum lma_compat.
	 * lma_self_fid and lma_flags are always available.
	 */
	__u32   lma_compat;
	/**
	 * Per-file incompat feature list. Lustre version should support all
	 * flags set in this field. The supported feature mask is available in
	 * LMA_INCOMPAT_SUPP.
	 */
	__u32   lma_incompat;
	/** FID of this inode */
	struct lu_fid  lma_self_fid;
};

struct lov_ost_data_v1
{							/* per-stripe data structure (little-endian)*/
	struct ost_id l_ost_oi; /* OST object ID */
	__u32 l_ost_gen;		/* generation of this l_ost_idx */
	__u32 l_ost_idx;		/* OST index in LOV (lov_tgt_desc->tgts) */
};

// #define LOV_MAGIC_V3	0x0BD30000
// #define 	LOV_MAGIC_V1   0x0BD10BD0
//This is struct for layout header
struct lov_mds_md_v1
{						   /* LOV EA mds/wire data (little-endian) */
	__u32 lmm_magic;	   /* magic number = LOV_MAGIC_V1 */
	__u32 lmm_pattern;	   /* LOV_PATTERN_RAID0, LOV_PATTERN_RAID1 */
	struct ost_id lmm_oi;  /* LOV object ID */
						   //struct ost_id;
	__u32 lmm_stripe_size; /* size of stripe in bytes */
	/* lmm_stripe_count used to be __u32 */
	__u16 lmm_stripe_count;				   /* num stripes in use for this object */
	__u16 lmm_layout_gen;				   /* layout generation number */
	struct lov_ost_data_v1 lmm_objects[0]; /* per-stripe data */
};

//This struct is for composite entry
struct lov_comp_md_entry_v1
{
	__u32 lcme_id;				  /* unique identifier of component */
	__u32 lcme_flags;			  /* LCME_FL_XXX */
	struct lu_extent lcme_extent; /* file extent for component */
	__u32 lcme_offset;			  /* offset of component blob in layout */
	__u32 lcme_size;			  /* size of component blob data */
	__u64 lcme_padding[2];
};

// #define LOV_MAGIC_COMP_V1 0x0BD40BD0
//This struct is for composite header
struct lov_comp_md_v1
{
	__u32 lcm_magic; /* LOV_USER_MAGIC_COMP_V1 */
	__u32 lcm_size;	 /* overall size including this struct */
	__u32 lcm_layout_gen;
	__u16 lcm_flags;
	__u16 lcm_entry_count;
	/* lcm_mirror_count stores the number of actual mirrors minus 1,
	 * so that non-flr files will have value 0 meaning 1 mirror. */
	__u16 lcm_mirror_count;
	__u16 lcm_padding1[3];
	__u64 lcm_padding2;
	struct lov_comp_md_entry_v1 lcm_entries[0];
} __attribute__((packed));

/*from inlcude/uapi/linux/lustre/lustre_user.h*/

#define lov_user_ost_data lov_user_ost_data_v1
struct lov_user_ost_data_v1
{							/* per-stripe data structure */
	struct ost_id l_ost_oi; /* OST object ID */
	__u32 l_ost_gen;		/* generation of this OST index */
	__u32 l_ost_idx;		/* OST index in LOV */
} __attribute__((packed));

#define lov_user_md lov_user_md_v1
struct lov_user_md_v1
{							/* LOV EA user data (host-endian) */
	__u32 lmm_magic;		/* magic number = LOV_USER_MAGIC_V1 */
	__u32 lmm_pattern;		/* LOV_PATTERN_RAID0, LOV_PATTERN_RAID1 */
	struct ost_id lmm_oi;	/* MDT parent inode id/seq (id/0 for 1.x) */
	__u32 lmm_stripe_size;	/* size of stripe in bytes */
	__u16 lmm_stripe_count; /* num stripes in use for this object */
	union
	{
		__u16 lmm_stripe_offset; /* starting stripe offset in
					   * lmm_objects, use when writing */
		__u16 lmm_layout_gen;	 /* layout generation number
					   * used when reading */
	};
	struct lov_user_ost_data_v1 lmm_objects[0]; /* per-stripe data */
} __attribute__((packed, __may_alias__));

/*from liblustre_layoutapi.c*/

/**
 * An Opaque data type abstracting the layout of a Lustre file.
 */
// struct llapi_layout {
// 	uint32_t	llot_magic; /* LLAPI_LAYOUT_MAGIC */
// 	uint32_t	llot_gen;
// 	uint32_t	llot_flags;
// 	bool		llot_is_composite;
// 	uint16_t	llot_mirror_count;
// 	/* Cursor pointing to one of the components in llot_comp_list */
// 	struct llapi_layout_comp *llot_cur_comp;
// 	struct list_head	  llot_comp_list;
// };

struct ext4_inode {
	__le16	i_mode;		/* File mode */
	__le16	i_uid;		/* Low 16 bits of Owner Uid */
	__le32	i_size_lo;	/* Size in bytes */
	__le32	i_atime;	/* Access time */
	__le32	i_ctime;	/* Inode Change time */
	__le32	i_mtime;	/* Modification time */
	__le32	i_dtime;	/* Deletion Time */
	__le16	i_gid;		/* Low 16 bits of Group Id */
	__le16	i_links_count;	/* Links count */
	__le32	i_blocks_lo;	/* Blocks count */
	__le32	i_flags;	/* File flags */
	union {
		struct {
			__le32  l_i_version;
		} linux1;
		struct {
			__u32  h_i_translator;
		} hurd1;
		struct {
			__u32  m_i_reserved1;
		} masix1;
	} osd1;				/* OS dependent 1 */
	__le32	i_block[EXT4_N_BLOCKS];/* Pointers to blocks */
	__le32	i_generation;	/* File version (for NFS) */
	__le32	i_file_acl_lo;	/* File ACL */
	__le32	i_size_high;
	__le32	i_obso_faddr;	/* Obsoleted fragment address */
	union {
		struct {
			__le16	l_i_blocks_high; /* were l_i_reserved1 */
			__le16	l_i_file_acl_high;
			__le16	l_i_uid_high;	/* these 2 fields */
			__le16	l_i_gid_high;	/* were reserved2[0] */
			__le16	l_i_checksum_lo;/* crc32c(uuid+inum+inode) LE */
			__le16	l_i_reserved;
		} linux2;
		struct {
			__le16	h_i_reserved1;	/* Obsoleted fragment number/size which are removed in ext4 */
			__u16	h_i_mode_high;
			__u16	h_i_uid_high;
			__u16	h_i_gid_high;
			__u32	h_i_author;
		} hurd2;
		struct {
			__le16	h_i_reserved1;	/* Obsoleted fragment number/size which are removed in ext4 */
			__le16	m_i_file_acl_high;
			__u32	m_i_reserved2[2];
		} masix2;
	} osd2;				/* OS dependent 2 */
	__le16	i_extra_isize;
	__le16	i_checksum_hi;	/* crc32c(uuid+inum+inode) BE */
	__le32  i_ctime_extra;  /* extra Change time      (nsec << 2 | epoch) */
	__le32  i_mtime_extra;  /* extra Modification time(nsec << 2 | epoch) */
	__le32  i_atime_extra;  /* extra Access time      (nsec << 2 | epoch) */
	__le32  i_crtime;       /* File Creation time */
	__le32  i_crtime_extra; /* extra FileCreationtime (nsec << 2 | epoch) */
	__le32  i_version_hi;	/* high 32 bits for 64-bit version */
};


/*
 * Structure of a directory entry
 */
#define EXT4_NAME_LEN 255

struct ext4_dir_entry {
	__le32	inode;			/* Inode number */
	__le16	rec_len;		/* Directory entry length */
	__le16	name_len;		/* Name length */
	char	name[EXT4_NAME_LEN];	/* File name */
};

/*
 * The new version of the directory entry.  Since EXT4 structures are
 * stored in intel byte order, and the name_len field could never be
 * bigger than 255 chars, it's safe to reclaim the extra byte for the
 * file_type field.
 */
struct ext4_dir_entry_2 {
	__le32	inode;			/* Inode number */
	__le16	rec_len;		/* Directory entry length */
	__u8	name_len;		/* Name length */
	__u8	file_type;
	char	name[EXT4_NAME_LEN];	/* File name */
};



/** The link ea holds 1 \a link_ea_entry for each hardlink */
#define LINK_EA_MAGIC 0x11EAF1DFUL
struct link_ea_header {
	__u32 leh_magic;
	__u32 leh_reccount;
	__u64 leh_len;	/* total size */
	__u32 leh_overflow_time;
	__u32 leh_padding;
};

/** Hardlink data is name and parent fid.
 * Stored in this crazy struct for maximum packing and endian-neutrality
 */
struct link_ea_entry {
        /** __u16 stored big-endian, unaligned */
        unsigned char      lee_reclen[2];
        unsigned char      lee_parent_fid[sizeof(struct lu_fid)];
        char               lee_name[0];
} __attribute__((packed));