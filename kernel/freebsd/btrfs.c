#include <sys/systm.h>
#include <sys/mount.h>
#include <sys/queue.h>
#include "btrfs_mount.h"
#include "btrfs_tree.h"

// @todo: key comparison for rb tree
int btrfs_find_key(struct btrfsmount_internal *bmp, struct btrfs_key *in_key, struct btrfs_key *out_key) {

    return(0);
}