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
struct b_stripe_list {
    btrfs_chunk_item_stripe val;
    LIST_ENTRY(b_stripe_list) entries;
};

LIST_HEAD(chunk_stripe_list, b_stripe_list);

struct b_chunk_bstrap {
    btrfs_key key;
    btrfs_chunk_item chunk_item;
    struct chunk_stripe_list *stripe_head;
    RB_ENTRY(b_chunk_bstrap) bt_entry;
};

//@todo: this is a dirty chunk comparison and I don't trust that it actually works.

static int chunk_cmp(struct b_chunk_bstrap *key_1, struct b_chunk_bstrap *key_2) {
        if ((key_1->key.offset > key_2->key.offset && key_1->key.offset < (key_2->key.offset + key_2->chunk_item.size)) ||
        ((key_1->key.offset + key_1->chunk_item.size) > key_2->key.offset &&
         (key_1->key.offset + key_1->chunk_item.size) < (key_2->key.offset + key_2->chunk_item.size))) {
        return 0;  // overlapping ranges
    }

    // key_1 is completely before key_2
    if (key_1->key.offset + key_1->chunk_item.size <= key_2->key.offset) {
        return -1;
    }

    // key_1 is completely after key_2
    if (key_1->key.offset >= key_2->key.offset + key_2->chunk_item.size) {
        return 1;
    }

    return 0;
}

RB_HEAD(sys_chunk_map, b_chunk_bstrap);
RB_GENERATE_STATIC(sys_chunk_map, b_chunk_bstrap, bt_entry, chunk_cmp); // comparing chunk key overlapping

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
    struct sys_chunk_map pm_chunk_bootstrap;
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