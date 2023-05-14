//
//  btrfs_mapping.c
//  macos-btrfs
//
//  Created by Yehia Hafez on 4/27/23.
//

#include "btrfs_mapping.h"
#include <sys/queue.h>

btrfs_chunk_tree_entry *btrfs_get_chunk_from_logical_addr(btrfs_inmem_vol *vol, uint64_t logical_addr) {
	btrfs_chunk_tree_entry *chunk_ret;

	LIST_FOREACH(chunk_ret, vol->ct_rec, ptrs) {
		if(logical_addr >= chunk_ret->key.start && logical_addr < (chunk_ret->key.start + chunk_ret->key.size)) {
			return chunk_ret;
		}
	}
	
	return NULL;
}

uint64_t btrfs_chunk_get_offset(btrfs_inmem_vol *vol, uint64_t logical_addr) {
	btrfs_chunk_tree_entry *ctr = btrfs_get_chunk_from_logical_addr(vol, logical_addr);
	if(ctr) return (ctr->value.offset + (logical_addr - ctr->key.start));
	return 0;
}
