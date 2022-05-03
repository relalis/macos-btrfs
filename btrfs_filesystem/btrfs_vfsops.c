//
//  btrfs_vfsops.c
//  macos-btrfs
//
//  Created by Yehia Hafez on 5/2/22.
//

#include <kern/locks.h>

lck_grp_t *btrfs_lock_grp;
lck_attr_t *btrfs_lock_attr;
