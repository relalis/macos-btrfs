
/* btrfs_filesystem.h
 * Copied from WinBtrfs, all credits to maharmstone (https://github.com/maharmstone/btrfs)
 *
 * Generic btrfs header file. Thanks to whoever it was who wrote
 * https://btrfs.wiki.kernel.org/index.php/On-disk_Format - you saved me a lot of time!
 *
 * I release this file, and this file only, into the public domain - do whatever
 * you want with it. You don't have to, but I'd appreciate if you let me know if you
 * use it anything cool - mark@harmstone.com. */

#pragma once

#include <stdint.h>

static const uint64_t superblock_addrs[] = { 0x10000, 0x4000000, 0x4000000000, 0x4000000000000, 0 };

#define BTRFS_MAGIC         0x4d5f53665248425f
#define MAX_LABEL_SIZE      0x100
#define SUBVOL_ROOT_INODE   0x100
#define BTRFS_LAST_FREE_OBJECTID    0xffffffffffffff00

#define TYPE_INODE_ITEM        0x01
#define TYPE_INODE_REF         0x0C
#define TYPE_INODE_EXTREF      0x0D
#define TYPE_XATTR_ITEM        0x18
#define TYPE_ORPHAN_INODE      0x30
#define TYPE_DIR_ITEM          0x54
#define TYPE_DIR_INDEX         0x60
#define TYPE_EXTENT_DATA       0x6C
#define TYPE_EXTENT_CSUM       0x80
#define TYPE_ROOT_ITEM         0x84
#define TYPE_ROOT_BACKREF      0x90
#define TYPE_ROOT_REF          0x9C
#define TYPE_EXTENT_ITEM       0xA8
#define TYPE_METADATA_ITEM     0xA9
#define TYPE_TREE_BLOCK_REF    0xB0
#define TYPE_EXTENT_DATA_REF   0xB2
#define TYPE_EXTENT_REF_V0     0xB4
#define TYPE_SHARED_BLOCK_REF  0xB6
#define TYPE_SHARED_DATA_REF   0xB8
#define TYPE_BLOCK_GROUP_ITEM  0xC0
#define TYPE_FREE_SPACE_INFO   0xC6
#define TYPE_FREE_SPACE_EXTENT 0xC7
#define TYPE_FREE_SPACE_BITMAP 0xC8
#define TYPE_DEV_EXTENT        0xCC
#define TYPE_DEV_ITEM          0xD8
#define TYPE_CHUNK_ITEM        0xE4
#define TYPE_TEMP_ITEM         0xF8
#define TYPE_DEV_STATS         0xF9
#define TYPE_SUBVOL_UUID       0xFB
#define TYPE_SUBVOL_REC_UUID   0xFC

#define BTRFS_ROOT_ROOT         1
#define BTRFS_ROOT_EXTENT       2
#define BTRFS_ROOT_CHUNK        3
#define BTRFS_ROOT_DEVTREE      4
#define BTRFS_ROOT_FSTREE       5
#define BTRFS_ROOT_TREEDIR      6
#define BTRFS_ROOT_CHECKSUM     7
#define BTRFS_ROOT_UUID         9
#define BTRFS_ROOT_FREE_SPACE   0xa
#define BTRFS_ROOT_DATA_RELOC   0xFFFFFFFFFFFFFFF7

#define BTRFS_COMPRESSION_NONE  0
#define BTRFS_COMPRESSION_ZLIB  1
#define BTRFS_COMPRESSION_LZO   2
#define BTRFS_COMPRESSION_ZSTD  3

#define BTRFS_ENCRYPTION_NONE   0

#define BTRFS_ENCODING_NONE     0

#define EXTENT_TYPE_INLINE      0
#define EXTENT_TYPE_REGULAR     1
#define EXTENT_TYPE_PREALLOC    2

#define BLOCK_FLAG_DATA         0x001
#define BLOCK_FLAG_SYSTEM       0x002
#define BLOCK_FLAG_METADATA     0x004
#define BLOCK_FLAG_RAID0        0x008
#define BLOCK_FLAG_RAID1        0x010
#define BLOCK_FLAG_DUPLICATE    0x020
#define BLOCK_FLAG_RAID10       0x040
#define BLOCK_FLAG_RAID5        0x080
#define BLOCK_FLAG_RAID6        0x100
#define BLOCK_FLAG_RAID1C3      0x200
#define BLOCK_FLAG_RAID1C4      0x400

#define FREE_SPACE_CACHE_ID     0xFFFFFFFFFFFFFFF5
#define EXTENT_CSUM_ID          0xFFFFFFFFFFFFFFF6
#define BALANCE_ITEM_ID         0xFFFFFFFFFFFFFFFC

#define BTRFS_INODE_NODATASUM   0x001
#define BTRFS_INODE_NODATACOW   0x002
#define BTRFS_INODE_READONLY    0x004
#define BTRFS_INODE_NOCOMPRESS  0x008
#define BTRFS_INODE_PREALLOC    0x010
#define BTRFS_INODE_SYNC        0x020
#define BTRFS_INODE_IMMUTABLE   0x040
#define BTRFS_INODE_APPEND      0x080
#define BTRFS_INODE_NODUMP      0x100
#define BTRFS_INODE_NOATIME     0x200
#define BTRFS_INODE_DIRSYNC     0x400
#define BTRFS_INODE_COMPRESS    0x800

#define BTRFS_SUBVOL_READONLY   0x1

#define BTRFS_COMPAT_RO_FLAGS_FREE_SPACE_CACHE          0x1
#define BTRFS_COMPAT_RO_FLAGS_FREE_SPACE_CACHE_VALID    0x2

#define BTRFS_INCOMPAT_FLAGS_MIXED_BACKREF      0x0001
#define BTRFS_INCOMPAT_FLAGS_DEFAULT_SUBVOL     0x0002
#define BTRFS_INCOMPAT_FLAGS_MIXED_GROUPS       0x0004
#define BTRFS_INCOMPAT_FLAGS_COMPRESS_LZO       0x0008
#define BTRFS_INCOMPAT_FLAGS_COMPRESS_ZSTD      0x0010
#define BTRFS_INCOMPAT_FLAGS_BIG_METADATA       0x0020
#define BTRFS_INCOMPAT_FLAGS_EXTENDED_IREF      0x0040
#define BTRFS_INCOMPAT_FLAGS_RAID56             0x0080
#define BTRFS_INCOMPAT_FLAGS_SKINNY_METADATA    0x0100
#define BTRFS_INCOMPAT_FLAGS_NO_HOLES           0x0200
#define BTRFS_INCOMPAT_FLAGS_METADATA_UUID      0x0400
#define BTRFS_INCOMPAT_FLAGS_RAID1C34           0x0800

#define BTRFS_SUPERBLOCK_FLAGS_SEEDING   0x100000000
#define BTRFS_SUPERBLOCK_SIZE 4096

#define BTRFS_ORPHAN_INODE_OBJID         0xFFFFFFFFFFFFFFFB

#define CSUM_TYPE_CRC32C        0
#define CSUM_TYPE_XXHASH        1
#define CSUM_TYPE_SHA256        2
#define CSUM_TYPE_BLAKE2        3

#pragma pack(push, 1)

typedef struct {
	uint8_t uuid[16];
} BTRFS_UUID;

typedef struct {
	uint64_t obj_id;
	uint8_t obj_type;
	uint64_t offset;
} KEY;

#define HEADER_FLAG_WRITTEN         0x000000000000001
#define HEADER_FLAG_SHARED_BACKREF  0x000000000000002
#define HEADER_FLAG_MIXED_BACKREF   0x100000000000000

/*!
 @struct tree_header
 @abstract
 @field csum Must match superblock values
 @field fs_uuid FS specific uuid
 @field address
 @field flags
 @field chunk_tree_uuid
 @field generation
 @field tree_id
 @field num_items
 @field level
 @discussion first four must match the super block. Remaining fields allowed to be different from the super.
 */
typedef struct {
	uint8_t csum[32];
	BTRFS_UUID fs_uuid;
	uint64_t address;
	uint64_t flags;
	BTRFS_UUID chunk_tree_uuid;
	uint64_t generation;
	uint64_t tree_id;
	uint32_t num_items;
	uint8_t level;
} tree_header;

/*!
 @struct leaf_node
 @abstract btrfs leaf item
 @field offset where to find the item in the leaf
 @field size size of item in the leaf
 @discussion A leaf is full of items. offset and size tell us where to find the item in the leaf (relative to the start of the data area).

 Size field of struct btrfs_item indicates how much data is stored.
 */
typedef struct {
	KEY key;
	uint32_t offset;
	uint32_t size;
} leaf_node;

/*!
 @struct internal_node
 @abstract holds pointers to other blocks
 @field key
 @field blockptr
 @field generation
 @discussion all non-leaf blocks are nodes, they hold only keys and pointers to other blocks
 */
typedef struct {
	KEY key;
	uint64_t address;
	uint64_t generation;
} internal_node;

/*!
 @struct btrfs_dev_item
 @abstract Represents a complete block device.
 @field dev_id the internal btrfs device id
 @field num_bytes size of the device
 @field bytes_used
 @field optimal_io_align optimal io alignment for this device
 @field optimal_io_width optimal io width for this device
 @field minimal_io_size minimal io size for this device
 @field type type and info about this device
 @field generation expected generation for this device
 @field start_offset starting byte of this partition on the device, to allow for stripe alignment in the future
 @field dev_group grouping information for allocation decisions
 @field seek_speed seek speed 0-100 where 100 is fastest
 @field bandwidth bandwidth 0-100 where 100 is fastest
 @field device_uuid btrfs generated uuid for this device
 @field fs_uuid uuid of FS that owns this device
 @discussion `dev_id` matches the dev_id in the filesystem's list of struct btrfs_devices.
 */
typedef struct {
	uint64_t dev_id;
	uint64_t num_bytes;
	uint64_t bytes_used;
	uint32_t optimal_io_align;
	uint32_t optimal_io_width;
	uint32_t minimal_io_size;
	uint64_t type;
	uint64_t generation;
	uint64_t start_offset;
	uint32_t dev_group;
	uint8_t seek_speed;
	uint8_t bandwidth;
	BTRFS_UUID device_uuid;
	BTRFS_UUID fs_uuid;
} DEV_ITEM;

#define SYS_CHUNK_ARRAY_SIZE 0x800
#define BTRFS_NUM_BACKUP_ROOTS 4

/*!
 @struct superblock_backup
 @abstract Used to hold a backup of a superblock
 @field root_tree_addr
 @field root_tree_generation
 @field chunk_tree_addr
 @field chunk_tree_generation
 @field extent_tree_addr
 @field extent_tree_generation
 @field fs_tree_addr
 @field fs_tree_generation
 @field dev_root_addr
 @field dev_root_generation
 @field csum_root_addr
 @field csum_root_generation
 @field total_bytes
 @field bytes_used
 @field num_devices
 @field reserved Reserved for future expansion
 @field root_level
 @field chunk_root_level
 @field extent_root_level
 @field fs_root_level
 @field dev_root_level
 @field csum_root_level
 @field reserved2 Reserved for future expansion, and as an alignment
 @todo complete documentation
 */
typedef struct {
	uint64_t root_tree_addr;
	uint64_t root_tree_generation;
	uint64_t chunk_tree_addr;
	uint64_t chunk_tree_generation;
	uint64_t extent_tree_addr;
	uint64_t extent_tree_generation;
	uint64_t fs_tree_addr;
	uint64_t fs_tree_generation;
	uint64_t dev_root_addr;
	uint64_t dev_root_generation;
	uint64_t csum_root_addr;
	uint64_t csum_root_generation;
	uint64_t total_bytes;
	uint64_t bytes_used;
	uint64_t num_devices;
	uint64_t reserved[4];
	uint8_t root_level;
	uint8_t chunk_root_level;
	uint8_t extent_root_level;
	uint8_t fs_root_level;
	uint8_t dev_root_level;
	uint8_t csum_root_level;
	uint8_t reserved2[10];
} superblock_backup;

/*!
 @struct superblock
 @abstract lists the main trees of the FS
 @field checksum Checksum of everything past this field (from 20 to 1000)
 @field uuid FS specific UUID, visible to user
 @field sb_phys_addr  physical address of this block (different for mirrors)
 @field flags
 @field magic btrfs Magic ("_BHRfS_M")
 @field generation btrfs generation number
 @field root_tree_addr  logical address of the root tree root
 @field chunk_tree_addr logical address of the chunk tree root
 @field log_tree_addr  logical address of the log tree root
 @field log_root_transid  will help find the new super based on the log root
 @field total_bytes
 @field bytes_used
 @field root_dir_objectid
 @field num_devices
 @field sector_size
 @field node_size
 @field leaf_size
 @field stripe_size
 @field n sys_chunk_array_size
 @field chunk_root_generation
 @field compat_flags
 @field compat_ro_flags only implementations that support the flags can write to the filesystem
 @field incompat_flags only implementations that support the flags can use the filesystem
 @field csum_type Btrfs currently uses the CRC32c little-endian hash function with seed -1
 @field root_level
 @field chunk_root_level
 @field log_root_level
 @field dev_item DEV_ITEM data for this device
 @field label may not contain '/' or '\\'
 @field cache_generation
 @field uuid_tree_generation
 @field metadata_uuid the UUID written into btree blocks
 @field reserved Reserved for future expansion
 @field sys_chunk_array Contains (KEY, CHUNK_ITEM) pairs for all SYSTEM chunks. This is needed to bootstrap the mapping from logical addresses to physical.
 @field backup Contain super_roots (4 btrfs_root_backup)
 @field reserved2 Reserved for future expansion
 @discussion The primary superblock is located at 0x1 0000 (6410 KiB). Mirror copies of the superblock are located at physical addresses 0x400 0000 (6410 MiB), 0x40 0000 0000 (25610 GiB), and 0x4 0000 0000 0000 (1 PiB), if these locations are valid. btrfs normally updates all superblocks, but in SSD mode it will update only one at a time. The superblock with the highest generation is used when reading.
 @note Btrfs only recognizes disks with a valid 0x1 0000 superblock; otherwise, there would be confusion with other filesystems.
 @todo expand documentation.
 */
typedef struct {
	uint8_t checksum[32];
	BTRFS_UUID uuid;
	uint64_t sb_phys_addr;
	uint64_t flags;
	uint64_t magic;
	uint64_t generation;
	uint64_t root_tree_addr;
	uint64_t chunk_tree_addr;
	uint64_t log_tree_addr;
	uint64_t log_root_transid;
	uint64_t total_bytes;
	uint64_t bytes_used;
	uint64_t root_dir_objectid;
	uint64_t num_devices;
	uint32_t sector_size;
	uint32_t node_size;
	uint32_t leaf_size;
	uint32_t stripe_size;
	uint32_t n;
	uint64_t chunk_root_generation;
	uint64_t compat_flags;
	uint64_t compat_ro_flags;
	uint64_t incompat_flags;
	uint16_t csum_type;
	uint8_t root_level;
	uint8_t chunk_root_level;
	uint8_t log_root_level;
	DEV_ITEM dev_item;
	char label[MAX_LABEL_SIZE];
	uint64_t cache_generation;
	uint64_t uuid_tree_generation;
	BTRFS_UUID metadata_uuid;
	uint64_t reserved[28];
	uint8_t sys_chunk_array[SYS_CHUNK_ARRAY_SIZE];
	superblock_backup backup[BTRFS_NUM_BACKUP_ROOTS];
	uint8_t reserved2[565];
} superblock;

#define BTRFS_TYPE_UNKNOWN   0
#define BTRFS_TYPE_FILE      1
#define BTRFS_TYPE_DIRECTORY 2
#define BTRFS_TYPE_CHARDEV   3
#define BTRFS_TYPE_BLOCKDEV  4
#define BTRFS_TYPE_FIFO      5
#define BTRFS_TYPE_SOCKET    6
#define BTRFS_TYPE_SYMLINK   7
#define BTRFS_TYPE_EA        8

#define BTRFS_CSUM_SIZE 32
#define BTRFS_FSID_SIZE 16
#define BTRFS_UUID_SIZE 16
#define BTRFS_LABEL_SIZE 256
#define BTRFS_SYSTEM_CHUNK_ARRAY_SIZE 2048
#define BTRFS_NUM_BACKUP_ROOTS 4

/*!
 @struct DIR_ITEM
 @abstract represents the header for a directory entry item used for both standard user-visible directories and internal directories used to manage named extended attributes
 @field key Key for the INODE_ITEM or ROOT_ITEM associated with this entry. Unused and zeroed out when the entry describes an extended attribute.
 @field transid transid of the transaction that created this entry.
 @field m Length of the extended attribute associated with this entry. 0 for standard directories.
 @field n Length of directory entry name.
 @field type
 @field name
 @discussion This structure  is associated with the DIR_ITEM and XATTR_ITEM items. This structure is not used outside of these items. It is immediately followed by the name. If it represents an extended attribute, the attribute data immediately follows the name.
 
 Types:
		BTRFS_FT_UNKNOWN = 0	The target object's type is unknown. Indicates corruption if used.
		BTRFS_FT_REG_FILE = 1		The target object is an INODE_ITEM representing a regular file.
		BTRFS_FT_DIR = 2 			The target object is an INODE_ITEM representing a directory or a ROOT_ITEM that will be presented as a directory.
		BTRFS_FT_CHRDEV = 3		The target object is an INODE_ITEM representing a character device node.
		BTRFS_FT_BLKDEV = 4		The target object is an INODE_ITEM representing a block device node.
		BTRFS_FT_FIFO = 5			The target object is an INODE_ITEM representing a FIFO device node.
		BTRFS_FT_SOCK = 6		The target object is an INODE_ITEM representing a socket device node.
		BTRFS_FT_SYMLINK = 7		The target object is an INODE_ITEM representing a symbolic link.
		BTRFS_FT_XATTR = 8 		This value is used on-disk and internally but is not user-visible.
 @todo expand documentation. WinBtrfs adds `name`, which is missing from the linux kernel header; possibly a struct packing method.
 */
typedef struct {
	KEY key;
	uint64_t transid;
	uint16_t m;
	uint16_t n;
	uint8_t type;
	char name[1];
} DIR_ITEM;

/*!
 @struct btrfs_timespec
 @field seconds
 @field nanoseconds
 */
typedef struct {
	uint64_t seconds;
	uint32_t nanoseconds;
} BTRFS_TIME;

/*!
 @struct INODE_ITEM
 @abstract This structure contains the information typically associated with a UNIX-style inode's stat(2) data. It is associated with the INODE_ITEM.
 @field generation nfs style generation number
 @field transid transid that last touched this inode
 @field st_size Size of the file in bytes.
 @field st_blocks Size allocated to this file, in bytes; Sum of the offset fields of all EXTENT_DATA items for this inode. For directories: 0.
 @field block_group Unused for normal inodes. Contains byte offset of block group when used as a free space inode.
 @field st_nlink Count of INODE_REF entries for the inode. When used outside of a file tree, 1.
 @field st_uid
 @field st_gid
 @field st_mode
 @field st_rdev
 @field flags Inode flags
 @field sequence modification sequence number for NFS
 @field reserved Space reserved for a future expansion
 @field atime st_atime
 @field ctime st_ctime
 @field mtime st_mtime
 @field otime Timestamp of inode creation.
 */
typedef struct {
	uint64_t generation;
	uint64_t transid;
	uint64_t st_size;
	uint64_t st_blocks;
	uint64_t block_group;
	uint32_t st_nlink;
	uint32_t st_uid;
	uint32_t st_gid;
	uint32_t st_mode;
	uint64_t st_rdev;
	uint64_t flags;
	uint64_t sequence;
	uint8_t reserved[32];
	BTRFS_TIME atime;
	BTRFS_TIME ctime;
	BTRFS_TIME mtime;
	BTRFS_TIME otime;
} INODE_ITEM;

/*!
 @struct ROOT_ITEM
 @abstract defines the root of a btree
 @field inode Several fields are initialized but only flags is interpreted at runtime.
 @field generation transid of the transaction that created this root.
 @field objid For file trees, the objectid of the root directory in this tree (always 256). Otherwise, 0.
 @field block_number The disk offset in bytes for the root node of this tree.
 @field byte_limit Unused. Always 0.
 @field bytes_used Unused.
 @field last_snapshot_generation The last transid of the transaction that created a snapshot of this root.
 @field flags
 @field num_references Originally indicated a reference count. In modern usage, it is only 0 or 1.
 @field drop_progress Contains key of last dropped item during subvolume removal or relocation. Zeroed otherwise.
 @field drop_level The tree level of the node described in drop_progress.
 @field root_level The height of the tree rooted at block_number.
 @field generation2 If equal to generation, indicates validity of the following fields. If the root is modified using an older kernel, this field and generation will become out of sync. This is normal and recoverable.
 @field uuid This subvolume's UUID.
 @field parent_uuid The parent's UUID (for use with send/receive).
 @field received_uuid The received UUID (for use with send/receive).
 @field ctransid The transid of the last transaction that modified this tree, with some exceptions (like the internal caches or relocation). Updated when an inode changes.
 @field otransid The transid of the transaction that created this tree.
 @field stransid The transid for the transaction that sent this subvolume. Nonzero for received subvolume.
 @field rtransid The transid for the transaction that received this subvolume. Nonzero for received subvolume.
 @field ctime Timestamp for ctransid.
 @field otime Timestamp for otransid.
 @field stime Timestamp for stransid.
 @field rtime Timestamp for rtransid.
 @field reserved For future expansion
 @discussion This structure holds defines the the root of a btree. It is associated with the ROOT_ITEM type. This structure is never used outside of this item.
 */
typedef struct {
	INODE_ITEM inode;
	uint64_t generation;
	uint64_t objid;
	uint64_t block_number;
	uint64_t byte_limit;
	uint64_t bytes_used;
	uint64_t last_snapshot_generation;
	uint64_t flags;
	uint32_t num_references;
	KEY drop_progress;
	uint8_t drop_level;
	uint8_t root_level;
	uint64_t generation2;
	BTRFS_UUID uuid;
	BTRFS_UUID parent_uuid;
	BTRFS_UUID received_uuid;
	uint64_t ctransid;
	uint64_t otransid;
	uint64_t stransid;
	uint64_t rtransid;
	BTRFS_TIME ctime;
	BTRFS_TIME otime;
	BTRFS_TIME stime;
	BTRFS_TIME rtime;
	uint64_t reserved[8];
} ROOT_ITEM;

/*!
 @struct CHUNK_ITEM
 @abstract contains the mapping from a virtualized usable byte range within the backing storage to a set of one or more stripes on individual backing devices
 @field size size of this chunk in bytes
 @field root_id objectid of the root referencing this chunk
 @field stripe_length
 @field type
 @field opt_io_alignment optimal io alignment for this chunk
 @field opt_io_width optimal io width for this chunk
 @field sector_size minimal io size for this chunk
 @field num_stripes 2^16 stripes is quite a lot, a second limit is the size of a single item in the btree
 @field sub_stripes sub stripes only matter for raid10
 @discussion This structure contains the mapping from a virtualized usable byte range within the backing storage to a set of one or more stripes on individual backing devices. In addition to the mapping, hints on optimal I/O parameters for this chunk. It is associated with CHUNK_ITEM items.
 
 Although the structure definition only contains one stripe member, CHUNK_ITEM items contain as many struct btrfs_stripe structures as specified in the num_stripes and sub_stripes fields.
 @todo Linux kernel header adds a `struct stripe` at the end of this structure, to store additional stripes in the item chunk.
 */
typedef struct {
	uint64_t size;
	uint64_t root_id;
	uint64_t stripe_length;
	uint64_t type;
	uint32_t opt_io_alignment;
	uint32_t opt_io_width;
	uint32_t sector_size;
	uint16_t num_stripes;
	uint16_t sub_stripes;
} CHUNK_ITEM;

/*!
 @struct CHUNK_ITEM_STRIPE
 @abstract This structure is used to define the backing device storage that compose a btrfs chunk. It is associated with the CHUNK_ITEM item. This structure is never used outside of this item.
 @field dev_id Device ID that contains this stripe
 @field offset Location of the start of the stripe, in bytes. Size is determined by the stripe_len field in struct btrfs_chunk.
 @field dev_uuid UUID of the device that contains this stripe. Used to confirm that the correct device has been retrieved.
 */
typedef struct {
	uint64_t dev_id;
	uint64_t offset;
	BTRFS_UUID dev_uuid;
} CHUNK_ITEM_STRIPE;

/*!
 @struct EXTENT_DATA
 @field generation transaction id that created this extent
 @field decoded_size max number of bytes to hold this extent in ram
 @field compression Compression type. Can be one of: BTRFS_COMPRESS_NONE (0), BTRFS_COMPRESS_ZLIB (1) and BTRFS_COMPRESS_LZO (2)
 @field encryption encryption type, currently always set to 0
 @field encoding Currently unused, allows for future expansion.
 @field type Type of extent; inline data or a real extent?
 @discussion decoded_size: when we split a compressed extent we can't know how big each of the resulting pieces will be.  So, this is an upper limit on the size of the extent in ram instead of an exact limit.
		compression, encryption: 32 bits for the various ways we might encode the data, including compression and encryption.  If any of these are set to something a given disk format doesn't understand it is treated like an incompat flag for reading and writing, but not for stat.
		offset: This allows a file extent to point into the middle of an existing extent on disk, sharing it between two snapshots (useful if some bytes in the middle of the extent have changed)
 */
typedef struct {
	uint64_t generation;
	uint64_t decoded_size;
	uint8_t compression;
	uint8_t encryption;
	uint16_t encoding;
	uint8_t type;
	uint8_t data[1];
} EXTENT_DATA;

typedef struct {
	uint64_t address;
	uint64_t size;
	uint64_t offset;
	uint64_t num_bytes;
} EXTENT_DATA2;

/*!
 @struct INODE_REF
 @abstract Allows you to find the btrfs_dir_item entries or the filename for a given inode
 @field index Index of the inode this item's referencing in the directory
 @field n length of the name, following this item
 @field name
 @discussion Indexed by (inode_number, BTRFS_INODE_REF_ITEM, parent_inode). Allows you to find the btrfs_dir_item entries or the filename for a given inode. There is one of these for each hard-linked copy of a file.
 */
typedef struct {
	uint64_t index;
	uint16_t n;
	char name[1];
} INODE_REF;

/*!
 @struct INODE_EXTREF
 @field dir
 @field index Index of the inode this item's referencing in the directory
 @field n length of the name
 @field name name of the inode
 @discussion From an inode to a name in a directory. Used if the regarding INODE_REF array ran out of space. This item requires the EXTENDED_IREF feature.
 */
typedef struct {
	uint64_t dir;
	uint64_t index;
	uint16_t n;
	char name[1];
} INODE_EXTREF;

#define EXTENT_ITEM_DATA            0x001
#define EXTENT_ITEM_TREE_BLOCK      0x002
#define EXTENT_ITEM_SHARED_BACKREFS 0x100

/*!
 @struct EXTENT_ITEM
 @abstract items in the extent btree are used to record the objectid of the owner of the block and the number of references
 @field refs
 @field generation
 @field flags
 */
typedef struct {
	uint64_t refcount;
	uint64_t generation;
	uint64_t flags;
} EXTENT_ITEM;

/*!
 @struct EXTENT_ITEM2
 @field firstitem
 @field level
 */
typedef struct {
	KEY firstitem;
	uint8_t level;
} EXTENT_ITEM2;

/*!
 @struct EXTENT_ITEM_V0
 @field refs
 */
typedef struct {
	uint32_t refcount;
} EXTENT_ITEM_V0;

typedef struct {
	EXTENT_ITEM extent_item;
	KEY firstitem;
	uint8_t level;
} EXTENT_ITEM_TREE;

typedef struct {
	uint64_t offset;
} TREE_BLOCK_REF;

/*!
 @struct EXTENT_DATA_REF
 @abstract 
 @field root
 @field objid
 @field offset
 @field count
 */
typedef struct {
	uint64_t root;
	uint64_t objid;
	uint64_t offset;
	uint32_t count;
} EXTENT_DATA_REF;

/*!
 @struct BLOCK_GROUP_ITEM
 @abstract Defines the location, properties, and usage of a block group.
 @field used The space used in this block group.
 @field chunk_tree The objectid of the chunk backing this block group.
 @field flags
 @discussion This structure defines the location, properties, and usage of a block group. It is associated with the BLOCK_GROUP_ITEM. This structure is never used outside of this item.
 
 Allocation Type
	 The type of storage this block group offers. SYSTEM chunks cannot be mixed, but DATA and METADATA chunks can be mixed.
	 BTRFS_BLOCK_GROUP_DATA [0x1]
	 BTRFS_BLOCK_GROUP_SYSTEM [0x2]
	 BTRFS_BLOCK_GROUP_METADATA [0x4]
 Replication Policy
	 The allocation policy this block group implements. Only one of the following flags may be set in any single block group. It is not possible to combine policies to create nested RAID levels beyond the RAID-10 support offered below. If no flags are specified, the block group is not replicated beyond a single, unstriped copy.
	 BTRFS_BLOCK_GROUP_RAID0 [0x8]		Striping (RAID-0)
	 BTRFS_BLOCK_GROUP_RAID1 [0x10]		Mirror on a separate device (RAID-1)
	 BTRFS_BLOCK_GROUP_DUP [0x20]		Mirror on a single device
	 BTRFS_BLOCK_GROUP_RAID10 [0x40]		Striping and mirroring (RAID-10)
	 BTRFS_BLOCK_GROUP_RAID5 [0x80]		Parity striping with single-disk fault tolerance (RAID-5)
	 BTRFS_BLOCK_GROUP_RAID6 [0x100]		Parity striping with double-disk fault tolerance (RAID-6)
 */
typedef struct {
	uint64_t used;
	uint64_t chunk_tree;
	uint64_t flags;
} BLOCK_GROUP_ITEM;

/*!
 @struct EXTENT_REF_V0
 @abstract old style backrefs item
 @field root
 @field generation
 @field objectid
 @field count
 */
typedef struct {
	uint64_t root;
	uint64_t gen;
	uint64_t objid;
	uint32_t count;
} EXTENT_REF_V0;

typedef struct {
	uint64_t offset;
} SHARED_BLOCK_REF;

typedef struct {
	uint64_t offset;
	uint32_t count;
} SHARED_DATA_REF;

#define FREE_SPACE_EXTENT 1
#define FREE_SPACE_BITMAP 2

/*!
 @struct FREE_SPACE_ENTRY
 @field offset
 @field bytes
 @field type
 */
typedef struct {
	uint64_t offset;
	uint64_t size;
	uint8_t type;
} FREE_SPACE_ENTRY;

/*!
 @struct FREE_SPACE_ITEM
 @field key
 @field generation
 @field num_entries
 @field num_bitmaps
 */
typedef struct {
	KEY key;
	uint64_t generation;
	uint64_t num_entries;
	uint64_t num_bitmaps;
} FREE_SPACE_ITEM;

/*!
 @struct ROOT_REF
 @abstract References a subvolume fs tree root. Used for both forward and backward root refs. The name of the tree is stored after the end of the struct
 @field dir Subtree ID
 @field index Directory sequence number of subtree entry
 @field n Length of subtree name
 @field name
 */
typedef struct {
	uint64_t dir;
	uint64_t index;
	uint16_t n;
	char name[1];
} ROOT_REF;

/*!
 @struct DEV_EXTENT
 @abstract dev extents record free space on individual devices
 @field chunktree Objectid of the CHUNK_TREE that owns this extent.
 @field objid Objectid of the CHUNK_ITEM that references this extent.
 @field address Offset of the CHUNK_ITEM that references this extent.
 @field length Length of this extent, in bytes.
 @field chunktree_uuid UUID of the CHUNK_TREE that owns this extent.
 @discussion This structure is used to map physical extents on an individual backing device to a chunk. This extent may be the only one for a particular chunk or one of several. It is associated with the DEV_ITEM item. This structure is never used outside of this item. The owner field points back to the chunk allocation mapping tree that allocated the extent.  The chunk tree uuid field is a way to double check the owner.
 */
typedef struct {
	uint64_t chunktree;
	uint64_t objid;
	uint64_t address;
	uint64_t length;
	BTRFS_UUID chunktree_uuid;
} DEV_EXTENT;

#define BALANCE_FLAGS_DATA          0x1
#define BALANCE_FLAGS_SYSTEM        0x2
#define BALANCE_FLAGS_METADATA      0x4

#define BALANCE_ARGS_FLAGS_PROFILES         0x001
#define BALANCE_ARGS_FLAGS_USAGE            0x002
#define BALANCE_ARGS_FLAGS_DEVID            0x004
#define BALANCE_ARGS_FLAGS_DRANGE           0x008
#define BALANCE_ARGS_FLAGS_VRANGE           0x010
#define BALANCE_ARGS_FLAGS_LIMIT            0x020
#define BALANCE_ARGS_FLAGS_LIMIT_RANGE      0x040
#define BALANCE_ARGS_FLAGS_STRIPES_RANGE    0x080
#define BALANCE_ARGS_FLAGS_CONVERT          0x100
#define BALANCE_ARGS_FLAGS_SOFT             0x200
#define BALANCE_ARGS_FLAGS_USAGE_RANGE      0x400

/*!
 @struct BALANCE_ARGS
 @abstract
 @field profiles
 @field devid device ID
 @field drange_start
 @field drange_end
 @field vrange_start
 @field vrange_end
 @field convert
 @field flags
 */
typedef struct {
	uint64_t profiles;

	union {
			uint64_t usage;
			struct {
					uint32_t usage_start;
					uint32_t usage_end;
			};
	};

	uint64_t devid;
	uint64_t drange_start;
	uint64_t drange_end;
	uint64_t vrange_start;
	uint64_t vrange_end;
	uint64_t convert;
	uint64_t flags;

	union {
			uint64_t limit;
			struct {
					uint32_t limit_start;
					uint32_t limit_end;
			};
	};

	uint32_t stripes_start;
	uint32_t stripes_end;
	uint8_t reserved[48];
} BALANCE_ARGS;

/*!
 @struct BALANCE_ITEM
 @abstract store balance parameters to disk so that balance can be properly resumed after crash or unmount
 @field flags BTRFS_BALANCE_*
 @field data
 @field metadata
 @field system
 @field reserved Reserved for future use
 */
typedef struct {
	uint64_t flags;
	BALANCE_ARGS data;
	BALANCE_ARGS metadata;
	BALANCE_ARGS system;
	uint8_t reserved[32];
} BALANCE_ITEM;

#define BTRFS_FREE_SPACE_USING_BITMAPS      1

/*!
 @struct FREE_SPACE_INFO
 @abstract Only used with space_cache v2. Every block group is represented in the free space tree by a free space info item, which stores some accounting information.
 @field count Number of extents that are tracking the free space for this block group
 @field flags Flags associated with this free_space_info (current it can be 0 or BTRFS_FREE_SPACE_BITMAP_KEY)
 @discussion This struct is keyed on (block_group_start, FREE_SPACE_INFO, block_group_length).
 */
typedef struct {
	uint32_t count;
	uint32_t flags;
} FREE_SPACE_INFO;

#define BTRFS_DEV_STAT_WRITE_ERRORS          0
#define BTRFS_DEV_STAT_READ_ERRORS           1
#define BTRFS_DEV_STAT_FLUSH_ERRORS          2
#define BTRFS_DEV_STAT_CORRUPTION_ERRORS     3
#define BTRFS_DEV_STAT_GENERATION_ERRORS     4

#define BTRFS_SEND_CMD_SUBVOL          1
#define BTRFS_SEND_CMD_SNAPSHOT        2
#define BTRFS_SEND_CMD_MKFILE          3
#define BTRFS_SEND_CMD_MKDIR           4
#define BTRFS_SEND_CMD_MKNOD           5
#define BTRFS_SEND_CMD_MKFIFO          6
#define BTRFS_SEND_CMD_MKSOCK          7
#define BTRFS_SEND_CMD_SYMLINK         8
#define BTRFS_SEND_CMD_RENAME          9
#define BTRFS_SEND_CMD_LINK           10
#define BTRFS_SEND_CMD_UNLINK         11
#define BTRFS_SEND_CMD_RMDIR          12
#define BTRFS_SEND_CMD_SET_XATTR      13
#define BTRFS_SEND_CMD_REMOVE_XATTR   14
#define BTRFS_SEND_CMD_WRITE          15
#define BTRFS_SEND_CMD_CLONE          16
#define BTRFS_SEND_CMD_TRUNCATE       17
#define BTRFS_SEND_CMD_CHMOD          18
#define BTRFS_SEND_CMD_CHOWN          19
#define BTRFS_SEND_CMD_UTIMES         20
#define BTRFS_SEND_CMD_END            21
#define BTRFS_SEND_CMD_UPDATE_EXTENT  22

#define BTRFS_SEND_TLV_UUID             1
#define BTRFS_SEND_TLV_TRANSID          2
#define BTRFS_SEND_TLV_INODE            3
#define BTRFS_SEND_TLV_SIZE             4
#define BTRFS_SEND_TLV_MODE             5
#define BTRFS_SEND_TLV_UID              6
#define BTRFS_SEND_TLV_GID              7
#define BTRFS_SEND_TLV_RDEV             8
#define BTRFS_SEND_TLV_CTIME            9
#define BTRFS_SEND_TLV_MTIME           10
#define BTRFS_SEND_TLV_ATIME           11
#define BTRFS_SEND_TLV_OTIME           12
#define BTRFS_SEND_TLV_XATTR_NAME      13
#define BTRFS_SEND_TLV_XATTR_DATA      14
#define BTRFS_SEND_TLV_PATH            15
#define BTRFS_SEND_TLV_PATH_TO         16
#define BTRFS_SEND_TLV_PATH_LINK       17
#define BTRFS_SEND_TLV_OFFSET          18
#define BTRFS_SEND_TLV_DATA            19
#define BTRFS_SEND_TLV_CLONE_UUID      20
#define BTRFS_SEND_TLV_CLONE_CTRANSID  21
#define BTRFS_SEND_TLV_CLONE_PATH      22
#define BTRFS_SEND_TLV_CLONE_OFFSET    23
#define BTRFS_SEND_TLV_CLONE_LENGTH    24

#define BTRFS_SEND_MAGIC "btrfs-stream"

typedef struct {
	uint8_t magic[13];
	uint32_t version;
} btrfs_send_header;

typedef struct {
	uint32_t length;
	uint16_t cmd;
	uint32_t csum;
} btrfs_send_command;

typedef struct {
	uint16_t type;
	uint16_t length;
} btrfs_send_tlv;

#pragma pack(pop)
