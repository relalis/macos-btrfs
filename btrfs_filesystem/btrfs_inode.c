//
//  btrfs_inode.c
//  macos-btrfs
//
//  Created by Yehia Hafez on 7/6/23.
//

#include "btrfs_inode.h"
#include "btrfs_volume.h"
#include "btrfs_parse.h"

errno_t btrfs_get_inode_ref(char* node, uint64_t inode_num, char* inode_payload) {
    errno_t err = -EIO;

    btrfs_tree_header header = btrfs_parse_header(node);
    return err;
}