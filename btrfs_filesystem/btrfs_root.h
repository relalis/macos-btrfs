//
//  btrfs_root.h
//  macos-btrfs
//
//  Created by Yehia Hafez on 03/26/24.
//

#ifndef _btrfs_root_h
#define _btrfs_root_h

#include "btrfs_volume.h"

//uint8_t *read_root_tree_root(int fd, uint64_t root_logical, btrfs_list *ct_rec);
uint8_t *btrfs_read_root(int fd, uint64_t chunk_root, btrfs_list *chunk_bootstrap);
uint8_t *read_fs_tree_root(int fd, superblock *sb_rec, uint8_t *root, btrfs_list *ct_rec);

#endif