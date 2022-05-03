//
//  btrfs.h
//  macos-btrfs
//
//  Created by Yehia Hafez on 1/8/22.
//

#ifndef _MACOS_BTRFS_H
#define _MACOS_BTRFS_H

#ifdef KERNEL

#include <sys/mount.h>
#include <libkern/OSMalloc.h>
#include <kern/locks.h>

// Lock group and attribute for lock initialization
//! @todo: define in btrfs_vfsops.c

__attribute__((visibility("hidden"))) extern lck_grp_t *btrfs_lock_grp;
__attribute__((visibility("hidden"))) extern lck_attr_t *btrfs_lock_attr;

// malloc tag
//! @todo: define in btrfs_vfsops.c

__attribute__((visibility("hidden"))) extern OSMallocTag btrfs_malloc_tag;

#include "btrfs_volume.h"

/*!
 @function BTRFS_MP
 @field mp VFS mount
 @return returns the BTRFS volume associated with the VFS mount @mp.
 */
static inline btrfs_volume *BTRFS_MP(mount_t mp) {
	return (btrfs_volume*) vfs_fsprivate(mp);
}

#endif // KERNEL

enum {
	//! @abstract One of these must be present, default is ON_ERRORS_CONTINUE|ON_ERRORS_FAIL_DIRTY.
	ON_ERRORS_PANIC = 0x01,
	ON_ERRORS_REMOUNT_RO = 0x02,
	ON_ERRORS_CONTINUE = 0x04,
	//! @abstract Optional, can be combined with any of the above.
	ON_ERRORS_RECOVER = 0x10,
	//! @abstract If the volume is dirty, and we attempted to mount read/write, return an error rather than force a read-only mount.
	ON_ERRORS_FAIL_DIRTY = 0x20, //! @discussion do we actually need this for btrfs?
};

/*!
 @struct The BTRFS mount options header passed from userspace
 @field fspec Path of device to mount, used by mount(2)
 @field acl Enable/disable support for Posix Access Control Lists. See acl(5)
 @field autodefrag Enable automatic file defragmentation
 @field barrier Ensure that all IO write operations make it through the device cache and are stored permanently when the filesystem is at its consistency checkpoint
 @field check_int These debugging options control the behavior of the integrity checking module
 @field clear_cache Force clearing and rebuilding of the disk space cache if something has gone wrong
 @field commit Set the interval of periodic transaction commit when data are synchronized to permanent storage. Higher interval values lead to larger amount of unwritten data, which has obvious consequences when the system crashes.
 @field compress Control BTRFS file data compression. Type may be specified as zlib, lzo, zstd or no (for no compression, used for remounting). If no type is specified, zlib is used.
 @discussion https://btrfs.readthedocs.io/en/latest/btrfs-man5.html
 @todo This is hacky as fuck and is just here as a placeholder. Fix and remove it later.
*/
typedef struct {
#ifndef KERNEL
	char *fspec;
#endif
	bool acl;
	bool autodefrag;
	bool barrier;
	union {
		struct {
			bool check_int;
			uint32_t check_int_data;
			uint32_t check_int_print_mask;
		};
	};
	bool clear_cache;
	int commit;
	union {
		struct {
			bool compress;
			int compress_type;
			bool compress_force;
			int compress_force_type;
		};
	};
	bool datacow;
	bool datasum;
	bool degraded;
	bool discard;
	bool enospc_debug;
	bool flushoncommit;
	int fragment;
	bool nologreplay;
	uint64_t max_inline;
	int metadata_ratio;
	bool norecovery;
	bool rescan_uuid_tree;
	bool rescue;
	bool skip_balance;
	bool space_cache;
	union {
		bool ssd;
		bool ssd_spread;
	};
	bool treelog;
	bool usebackuproot;
	bool user_subvol_rm_allowed;
} __attribute__((__packed__)) btrfs_mount_options_header;

typedef uint32_t BTRFS_MNT_OPTS; //!@discussion this is little-endian in the NTFS source, but I'm guessing that was for PowerPC compatibility. Since neither Apple Silicon nor Intel are big-endian, we're good.

#endif // _MACOS_BTRFS_H
