//
//  btrfs_mapping.h
//  macos-btrfs
//
//  Created by Yehia Hafez on 4/27/23.
//

#ifndef btrfs_mapping_h
#define btrfs_mapping_h

#include "btrfs_volume.h"

btrfs_chunk_tree_entry *btrfs_get_chunk_from_logical_addr(btrfs_inmem_vol *vol, uint64_t logical_addr);
uint64_t btrfs_chunk_get_offset(btrfs_inmem_vol *vol, uint64_t logical_addr);

#endif /* btrfs_mapping_h */
