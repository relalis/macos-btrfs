//
//  btrfs_parse.h
//  macos-btrfs
//
//  Created by Yehia Hafez on 2/12/23.
//

#ifndef parse_h
#define parse_h

#include "btrfs_volume.h"

btrfs_tree_header btrfs_parse_header(char *buffer);
btrfs_leaf_list_record *btrfs_parse_leaf(char *buffer);
btrfs_internal_list_record *btrfs_parse_node(char *buffer);

#endif /* parse_h */
