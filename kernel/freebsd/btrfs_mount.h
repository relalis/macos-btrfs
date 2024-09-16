#ifndef _BTRFS_MOUNT_H
#define _BTRFS_MOUNT_H

#include <sys/types.h>
#include <sys/tree.h>
#include <sys/lock.h>
#include "btrfs.h"

// BTRFS in Linux is represented (effectively) in a red-black tree
// this is not actually necessary for this implementation (any binary search tree
// would work) but for the sake of exercise, we'll simply use RB trees.

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

    struct lock pm_btrfslock;                   // protects allocations
    struct lock pm_checkpath_lock;              // protects checkpath result
    uint32_t pm_sector_size;                    // BTRFS sector size
    uint64_t pm_superblock_physical_addr;       // store superblock addr in case we're using a backup sblock
    uint64_t pm_root_addr;                      // logical address of root tree root

    RB_HEAD(sys_chunk_map, btrfs_chunk_tree_keypair) pm_chunkmap;           // head of the rb tree
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