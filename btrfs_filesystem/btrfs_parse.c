//
//  btrfs_parse.c
//  macos-btrfs
//
//  Created by Yehia Hafez on 2/12/23.
//

#include "btrfs_parse.h"

#include <string.h>
#include <libkern/libkern.h>

/// @todo: this was testing on aleena, cleanup for mac
btrfs_tree_header rbt_parse_header(char *buffer) {
	btrfs_tree_header header_from_buffer; // = OSMalloc(sizeof(btrfs_tree_header), new_header_tag);

	memcpy(&header_from_buffer, buffer, sizeof(btrfs_tree_header));

	return header_from_buffer;
}

void rbt_parse_leaf_node(char *buffer) {

	btrfs_tree_header header = rbt_parse_header(buffer);

	for(int i = 0; i < header.num_items; ++i) {
		btrfs_leaf_node item = rbt_parse_leaf(buffer + sizeof(btrfs_leaf_node) * i);
		printf("item.offset = %X\n", item.offset);
	}
}

btrfs_leaf_node rbt_parse_leaf(char *buffer) {
	btrfs_leaf_node return_leaf;
	memcpy(&return_leaf, buffer, sizeof(btrfs_leaf_node));
	return return_leaf;
}

/// @todo: i forgot how i did this on aleena

void *rbt_parse_node(char *buffer) {
	btrfs_tree_header header = rbt_parse_header(buffer);
	for(int i = 0; i < header.num_items; ++i) {
		printf("parse_node() item %d is of type: %d\n", i, ((btrfs_internal_node*)(buffer + sizeof(btrfs_tree_header) + (sizeof(btrfs_internal_node) * i) ))->key.obj_type);
	}
	return NULL;
}
