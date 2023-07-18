//
//  btrfs_parse.c
//  macos-btrfs
//
//  Created by Yehia Hafez on 2/12/23.
//

#include "btrfs_parse.h"
#include <IOKit/IOLib.h>

btrfs_tree_header btrfs_parse_header(char *buffer) {
	btrfs_tree_header header;
	memcpy(&header, buffer, sizeof(btrfs_tree_header));
	return header;
}

btrfs_leaf_list_record *btrfs_parse_leaf(char *buffer) {
	btrfs_leaf_list_record *leaves = IOMallocZero(sizeof(btrfs_leaf_list_record));

	if(leaves == NULL) {
		DMESG_LOG("failed to allocate mem in parse_leaf()");
		return NULL;
	}

	LIST_INIT(leaves);

	// everything starts with a header
	btrfs_tree_header header = btrfs_parse_header(buffer);

	for(int i = 0; i < header.num_items; ++i) {
		btrfs_leaf_node_list_entry *new_leaf = IOMallocZero(sizeof(btrfs_leaf_node_list_entry));
		if(new_leaf == NULL) {
			DMESG_LOG("failed to allocate mem for a leaf");
			goto exit;
		}
		memcpy(&new_leaf->node, buffer + sizeof(btrfs_tree_header) + (i * sizeof(btrfs_leaf_node)), sizeof(btrfs_leaf_node));
		LIST_INSERT_HEAD(leaves, new_leaf, ptrs);
	}

exit:
	return leaves;
}

btrfs_internal_list_record *btrfs_parse_node(char *buffer) {
	btrfs_internal_list_record *nodes = IOMallocZero(sizeof(btrfs_internal_list_record));
	LIST_INIT(nodes);
	if(nodes == NULL) {
		DMESG_LOG("failed to allocate mem in parse_node()");
		return NULL;
	}

	btrfs_tree_header header = btrfs_parse_header(buffer);

	for(int i = 0; i < header.num_items; ++i) {
		btrfs_internal_node_list_entry *new_node = IOMallocZero(sizeof(btrfs_internal_node_list_entry));
		if(new_node == NULL) {
			DMESG_LOG("failed to allocate mem for a leaf");
			goto exit;
		}
		memcpy(&new_node->node, buffer + sizeof(btrfs_tree_header) + (i * sizeof(btrfs_internal_node)), sizeof(btrfs_internal_node));
		LIST_INSERT_HEAD(nodes, new_node, ptrs);
		DMESG_LOG("btrfs_parse_node() item %d is of type: %d", i, new_node->node.key.obj_type);
	}

exit:
	return nodes;
}
