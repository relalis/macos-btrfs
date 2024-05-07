//
//  btrfs_mapping.c
//  macos-btrfs
//
//  Created by Yehia Hafez on 4/27/23.
//

#include "btrfs_mapping.h"
#include "btrfs_chunk.h"

btrfs_chunk_tree_entry *btrfs_read_logical(btrfs_list *chunk_tree, uint64_t logical_addr) {
	for(btrfs_list *current = chunk_tree->head; current != NULL; current = current->child) {
		btrfs_chunk_tree_entry *cur_node = current->node;
		if(logical_addr >= cur_node->key.start && logical_addr < (cur_node->key.start + cur_node->key.size)) {
			return cur_node;
		}
	}
	return NULL;
}

uint64_t btrfs_chunk_get_offset(btrfs_list *chunk_tree, uint64_t logical_addr) {
	btrfs_chunk_tree_entry *ctr = btrfs_read_logical(chunk_tree, logical_addr);

	if(ctr) {
		uint64_t ret_val = ctr->offset + (logical_addr - ctr->key.start);
		return ret_val;
	}

	return 0;
}