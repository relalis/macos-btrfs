//
//  btrfs_mapping.h
//  macos-btrfs
//
//  Created by Yehia Hafez on 04/27/23.
//

#ifndef btrfs_mapping_h
#define btrfs_mapping_h

#include "btrfs_volume.h"

typedef struct _btrfs_chunk_tree_entry {
	struct {uint64_t start; uint64_t size;} key;
	uint64_t offset;
} btrfs_chunk_tree_entry;


btrfs_chunk_tree_entry *btrfs_read_logical(btrfs_list *chunk_tree, uint64_t logical_addr);
uint64_t btrfs_chunk_get_offset(btrfs_list *chunk_tree, uint64_t logical_addr);

#endif /* btrfs_mapping_h */
