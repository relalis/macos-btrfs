//
//  btrfs_dir_item.c
//  macos-btrfs
//
//  Created by Yehia Hafez on 7/7/23.
//

#include "btrfs_dir_item.h"
#include "btrfs_parse.h"
#include <IOKit/IOLib.h>
#include <sys/buf.h>

// todo: btrfs_path from linux src

/// so far this is copied over from btrfs_vfsops.c's btrfs_vfs_root(), but should be a few key differences
/// for starters, the header.level isn't guaranteed to be 0. Context/cred should be handled better. crc32c
/// isn't added (yet).
btrfs_dir_item *btrfs_lookup_dir_item(struct mount *mp, const char *name, int name_len, kauth_cred_t cred) {
    btrfs_inmem_vol *vol = vfs_fsprivate(mp);
    buf_t bp;
    char *tree_root_node = IOMallocZero(vol->sb_rec.node_size);
    buf_meta_bread(vol->dev_vn, vol->fs_tree_root_off / vol->sb_rec.sector_size, vol->sb_rec.node_size, cred, &bp);
    buf_setflags(bp, B_NOCACHE);
    memcpy(tree_root_node, buf_dataptr(bp), vol->sb_rec.node_size);
    buf_brelse(bp);

    btrfs_tree_header header = btrfs_parse_header(tree_root_node);
    btrfs_leaf_list_record *items = btrfs_parse_leaf(tree_root_node);


    btrfs_dir_item item;
    return &item;
}