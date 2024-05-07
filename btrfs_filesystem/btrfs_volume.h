//
//  btrfs_volume.h
//  macos-btrfs
//
//  Created by Yehia Hafez on 05/02/22.
//

#ifndef btrfs_volume_h
#define btrfs_volume_h

#include <sys/mount.h>
#include <sys/types.h>
#include <libkern/OSAtomic.h>
#include <kern/locks.h>

typedef struct _btrfs_volume btrfs_volume;

#include "btrfs_filesystem.h"

typedef struct _btrfs_list_entry {
	void *node;
	struct _btrfs_list_entry *child;
	struct _btrfs_list_entry *parent;
	struct _btrfs_list_entry *head;
	struct _btrfs_list_entry *tail;
} btrfs_list;

// defined in btrfs_list.c
errno_t btrfs_list_push(btrfs_list *src, btrfs_list **dest);
errno_t btrfs_list_destroy(btrfs_list **list, size_t node_size);

typedef struct _btrfs_volume {
	/// @discussion the superblock is stored in-memory for now, as this is intended to be read-only
	superblock sb_rec;		/// BTRFS superblock
	int64_t num_blocks;		/// number of 4096-sized blocks in disk

	uid_t uid;
	gid_t gid;
	mode_t fmask;
	mode_t gmask;


	btrfs_root_item fs_tree_root;	/// fs_tree_root, bootstrapped
	uint64_t fs_tree_root_off;		/// physical offset of fs_tree_root

	uint8_t is_root_attaching;	/// true if attaching a root vnode
	uint8_t is_root_waiting;	/// true if waiting for attach to complete

#ifdef KERNEL
	mount_t mp;				/// vfs mount
	dev_t dev;				/// device number
	vnode_t dev_vn;			/// device vnode; use refcnt on it
	lck_mtx_t *mtx_root;	/// root mutex lock for protection
	vnode_t rootvp;				/// root vnode 
#endif

	btrfs_list *ct_rec;	/// bootstrapped chunk tree

} btrfs_inmem_vol;

#endif /* btrfs_volume_h */
