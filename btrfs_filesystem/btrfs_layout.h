//
//  btrfs_layout.h
//  macos-btrfs
//
//  Created by Yehia Hafez on 5/24/21.
//

#ifndef btrfs_layout_h
#define btrfs_layout_h

#include "btrfs_types.h"

#define BTRFS_MAGIC "_BHRfS_M"
#define BTRFS_CSUM_SIZE 32
#define BTRFS_FSID_SIZE 16
#define BTRFS_UUID_SIZE 16
#define BTRFS_LABEL_SIZE 256
#define BTRFS_SYSTEM_CHUNK_ARRAY_SIZE 2048
#define BTRFS_NUM_BACKUP_ROOTS 4

enum btrfs_csum_type {
	BTRFS_CSUM_TYPE_CRC32	= 0,
};

#pragma pack(push, 8)
typedef struct {
	le32 data1;					// The first eight hexadecimal digits of the GUID.
	le16 data2;					// The first group of four hexadecimal digits.
	le16 data3;					// The second group of four hexadecimal digits.
	u8 data4[8];				// The first two bytes are the third group of four
								// hexadecimal digits.  The remaining six bytes are the
								// final 12 hexadecimal digits.
} GUID_STRUCTURE;

typedef struct {
	le64 tree_root;
	le64 tree_root_gen;
	
	le64 chunk_root;
	le64 chunk_root_gen;

	le64 extent_root;
	le64 extent_root_gen;

	le64 fs_root;
	le64 fs_root_gen;

	le64 dev_root;
	le64 dev_root_gen;

	le64 csum_root;
	le64 csum_root_gen;

	le64 total_bytes;
	le64 bytes_used;
	le64 num_devices;

	le64 unused_64[4];	// future expansion

	u8 tree_root_level;
	u8 chunk_root_level;
	u8 extent_root_level;
	u8 fs_root_level;
	u8 dev_root_level;
	u8 csum_root_level;

	u8 unused_8[10];	// future expansion; alignment
} BTRFS_ROOT_BACKUP;

typedef struct {
	le64 id;
	le64 total_size;
	le64 total_used;
	le32 io_align;
	le32 io_width;
	le32 min_sector_size;
	le64 type;
	le64 generation;
	le64 start_offset;
	le64 group;
	u8 seek_speed;
	u8 bandwidth;
	u8 uuid[BTRFS_UUID_SIZE];
	u8 fsid[BTRFS_FSID_SIZE];
} BTRFS_DEV_ITEM;

typedef struct {
	u8 csum[BTRFS_CSUM_SIZE];			// Checksum of everything past this field (from 20 to 1000)
	u8 fsid[BTRFS_FSID_SIZE];			// Filesystem UUID
	le64 sb_addr;						// physical address of this block (different for mirrors)
	le64 flags;
	u8 sb_magic[0x8];					// magic ("_BHRfS_M")
	le64 sb_gen;						// btrfs generation
	le64 root_tree_addr;				// logical address of the root tree root
	le64 chunk_tree_addr; 				// logical address of the chunk tree root
	le64 log_tree_addr;					// logical address of the log tree root
	le64 log_root_transid;
	le64 total_bytes;
	le64 bytes_used;
	le64 root_dir_objectid;				// usually 6
	le64 num_devices;
	le32 sectorsize;
	le32 nodesize;
	le32 leafsize;
	le32 stripesize;
	le32 sys_chunk_array_size;
	le64 chunk_root_generation;
	le64 compat_flags;
	le64 compat_ro_flags;				// only implementations that support the flags can write to the filesystem
	le64 incompat_flags;				// only implementations that support the flags can use the filesystem
	btrfschar csum_type;				// Btrfs currently uses the CRC32c little-endian hash function with seed -1
	u8 root_level;
	u8 chunk_root_level;
	u8 log_root_level;
	BTRFS_DEV_ITEM device;				// data for this device
	char label[BTRFS_LABEL_SIZE];		// may not contain '/' or '\\'
	le64 cache_generation;
	le64 uuid_tree_generation;			// UUID Tree and Generation
	u8 metadata_uuid[BTRFS_FSID_SIZE]; 	// UUID written into btree blocks
	le64 future_reserved[28];			// Reserved for future expansion

	// (n bytes valid) Contains (KEY, CHUNK_ITEM) pairs for all SYSTEM chunks;
	// needed to bootstrap the mapping from logical addresses to physical
	u8 sys_chunk_array[BTRFS_SYSTEM_CHUNK_ARRAY_SIZE];

	BTRFS_ROOT_BACKUP super_roots[BTRFS_NUM_BACKUP_ROOTS];
} BTRFS_SUPERBLOCK;

typedef struct {
	u8 csum[20];				// Checksum of everything past this field (from 20 to node end)
	u8 uuid[10];				// Filesystem UUID
	le64 log_addr;				// Logical address of this node
	u8 flags[7];
	bool new_fs;				// always 1 for new filesystems; 0 indicates an old filesystem.
	u8 chunk_tree_uuid[10];
	le64 generation;
	le64 parent_tree_id;		// The ID of the tree that contains this node
	le32 num_items;				// Number of items
	u8 level;					// Level (0 for leaf nodes)
} BTRFS_HEADER;

#pragma pack(pop)

static inline bool __btrfs_check_magic(le64 x, le64 r) {return (x == r);}
static inline bool __btrfs_check_magicp(le64 *p, le64 r) {return (*p == r);}

#endif /* btrfs_layout_h */
