#ifndef _btrfs_leaf_h
#define _btrfs_leaf_h

#include "btrfs_volume.h"

errno_t btrfs_parse_leaf(uint8_t *buffer, btrfs_list **dest);
errno_t btrfs_parse_node(uint8_t *buffer, btrfs_list **dest);

#endif