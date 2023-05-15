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
#include <sys/queue.h>
#include <libkern/OSAtomic.h>
#include <kern/locks.h>

typedef struct _btrfs_volume btrfs_volume;

#include "btrfs_filesystem.h"

typedef struct _btrfs_chunk_tree_entry {
	struct {uint64_t start; uint64_t size;} key;
	struct {uint64_t offset;} value;
	LIST_ENTRY(_btrfs_chunk_tree_entry) ptrs;
} btrfs_chunk_tree_entry;

typedef struct _btrfs_leaf_node_entry {
	btrfs_leaf_node node;
	LIST_ENTRY(_btrfs_leaf_node_entry) ptrs;
} btrfs_leaf_node_list_entry;

typedef struct _btrfs_internal_node_entry {
	btrfs_internal_node node;
	LIST_ENTRY(_btrfs_internal_node_entry) ptrs;
} btrfs_internal_node_list_entry;


typedef LIST_HEAD(, _btrfs_chunk_tree_entry) btrfs_chunk_tree_record;
typedef LIST_HEAD(, _btrfs_leaf_node_entry) btrfs_leaf_list_record;
typedef LIST_HEAD(, _btrfs_internal_node_entry) btrfs_internal_list_record;

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
	
	btrfs_root_item fs_tree_root;	/// fs_tree_root, bootstrapped
	uint64_t fs_tree_root_off;		/// physical offset of fs_tree_root

	
	
	btrfs_chunk_tree_record *ct_rec;
} btrfs_inmem_vol;

#endif /* btrfs_volume_h */
