#ifndef _BTRFS_MOUNT_H
#define _BTRFS_MOUNT_H

#include <sys/types.h>
#include <sys/queue.h>
#include <sys/tree.h>
#include <sys/lock.h>
#include "btrfs.h"

// BTRFS in Linux is represented (effectively) in a red-black tree
// this is not actually necessary for this implementation (any binary search tree
// would work) but for the sake of exercise, we'll simply use RB trees.

// we're not implementing the RB tree structure correctly, but cleanup will come later
// @todo: use generic RB trees rather than specific trees for key/chunk_item pairs in sys_chunk_array

struct b_chunk_bstrap {
    struct btrfs_key key;
    struct btrfs_chunk_item chunk_item;
    // only reading one stripe- no RAID for this implementation (for now)
    struct btrfs_chunk_item_stripe stripe;
    LIST_ENTRY(b_chunk_bstrap) entries;
};

LIST_HEAD(sys_chunk_bootstrap, b_chunk_bstrap) chunk_bootstrap_list;

struct btrfsmount_internal {
    struct mount *pm_mountp;                    // vfs mount struct
    struct g_consumer *pm_cp;
    struct bufobj *pm_bo;
    uid_t pm_uid;                               // uid for file owner
    gid_t pm_gid;                               // gid for file owner
    mode_t pm_mask;                             // mask for files
    mode_t pm_dirmask;                          // mask for dirs
    struct vnode *pm_devvp;                     // vnode for character device we're mounting
    struct vnode *pm_odevvp;                    // msdosfs refers to this as the "real devfs vnode"
                                                // I have yet to understand why, or its purpose
    struct cdev *pm_dev;                        // character device we're mounting

    struct lock pm_btrfslock;                   // protects allocations
    struct lock pm_checkpath_lock;              // protects checkpath result
    struct btrfs_superblock pm_superblock;             // superblock struct

    struct sys_chunk_bootstrap *pm_chunk_bootstrap;
};

struct btrfs_args {
    char *fspec;                    // blocks special device holding the fs to mount
    struct oexport_args export;     // network export information
    uid_t uid;
    gid_t gid;
    mode_t mask;
    mode_t dirmask;
    int flags;
};

#endif // _BTRFS_MOUNT_H