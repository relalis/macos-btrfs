//
//  btrfs_chunk.h
//  macos-btrfs
//
//  Created by Yehia Hafez on 03/26/24.
//

#ifndef btrfs_chunk
#define btrfs_chunk


#include "btrfs_filesystem.h"
#include "btrfs_volume.h"

errno_t get_chunk_tree_bootstrap(superblock *sb_rec, btrfs_list **dest);
errno_t btrfs_read_chunk_tree(int fd, uint8_t *root, btrfs_list **dest_chunk_tree);

#endif