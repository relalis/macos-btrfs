//
//  btrfs_volume.h
//  macos-btrfs
//
//  Created by Yehia Hafez on 5/2/22.
//

#ifndef btrfs_volume_h
#define btrfs_volume_h

#include <sys/mount.h>
#include <sys/types.h>
#include <libkern/OSAtomic.h>
#include <kern/locks.h>

typedef struct _btrfs_volume btrfs_volume;

#include "btrfs_filesystem.h"

typedef struct _btrfs_volume {
	mount_t mp;				/// vfs mount
	dev_t dev;				/// device number
	vnode_t dev_vn;			/// device vnode
	/// @discussion the superblock is stored in-memory for now, as this is intended to be read-only
	superblock sb_rec;		/// BTRFS superblock
	int64_t num_blocks;		/// number of 4096-sized blocks in disk

	uid_t uid;
	gid_t gid;
	mode_t fmask;
	mode_t gmask;
} btrfs_inmem_vol;

#endif /* btrfs_volume_h */
