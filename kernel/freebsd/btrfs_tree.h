#ifndef _BTRFS_TREE_OPS_H
#define _BTRFS_TREE_OPS_H

#include <sys/tree.h>

int btrfs_find_key(struct btrfsmount_internal *bmp, struct btrfs_key *in_key, struct btrfs_key *out_key);

#endif