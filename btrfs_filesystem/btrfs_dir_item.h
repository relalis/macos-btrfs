//
//  btrfs_dir_item.h
//  macos-btrfs
//
//  Created by Yehia Hafez on 7/7/23.
//

#ifndef _BTRFS_DIR_ITEM_H
#define _BTRFS_DIR_ITEM_H

#include "btrfs_volume.h"

btrfs_dir_item *btrfs_lookup_dir_item(struct mount *mp, const char *name, int name_len, kauth_cred_t cred);

#endif //_BTRFS_DIR_ITEM_H