//
//  btrfs_parse.h
//  macos-btrfs
//
//  Created by Yehia Hafez on 2/12/23.
//

#ifndef parse_h
#define parse_h

#include "btrfs_filesystem.h"

btrfs_tree_header rbt_parse_header(char *buffer);
btrfs_leaf_node rbt_parse_leaf(char *buffer);

void rbt_parse_leaf_node(char *buffer);
void *rbt_parse_node(char *buffer);

#endif /* parse_h */
