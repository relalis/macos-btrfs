//
//  btrfs_inode.h
//  macos-btrfs
//
//  Created by Yehia Hafez on 7/6/23.
//

#ifndef btrfs_inode_h
#define btrfs_inode_h

#include "btrfs_filesystem.h"
#include <sys/queue.h>

typedef struct _btrfs_inode_reference {
    TAILQ_ENTRY(_btrfs_inode_reference) ptrs;
    int test;
} btrfs_inode_reference;

typedef TAILQ_HEAD(, _btrfs_inode_reference) btrfs_inode_ref_record;

errno_t btrfs_get_inode_ref(char* node, uint64_t inode_num, char* inode_payload);

#endif //btrfs_inode_h