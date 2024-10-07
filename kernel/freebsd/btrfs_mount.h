/*
Copyright (c) 2024, Yehia Hafez 
All rights reserved. 

Redistribution and use in source and binary forms, with or without 
modification, are permitted provided that the following conditions are met: 

 * Redistributions of source code must retain the above copyright notice, 
   this list of conditions and the following disclaimer. 
 * Redistributions in binary form must reproduce the above copyright 
   notice, this list of conditions and the following disclaimer in the 
   documentation and/or other materials provided with the distribution. 

THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND ANY 
EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE FOR ANY 
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT 
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY 
OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH 
DAMAGE.
*/

#ifndef _BTRFS_MOUNT_H
#define _BTRFS_MOUNT_H

#include <sys/types.h>
#include <sys/lock.h>
#include <sys/lockmgr.h>
#include <sys/tree.h>
#include <sys/queue.h>
#include "btrfs_filesystem.h"

// BTRFS in Linux is represented in a red-black tree
// @todo: use generic RB trees rather than lists for key/chunk_item pairs in sys_chunk_array

struct b_chunk_list {
    struct btrfs_key key;
    struct btrfs_chunk_item chunk_item;
    // only reading one stripe- no RAID for this implementation (for now)
    struct btrfs_chunk_item_stripe chunk_stripe;
    LIST_ENTRY(b_chunk_list) entries;
};

// Linux kernel has a helpful struct (btrfs/fs.h) that holds pointers to all the roots
// we will encounter. Seems like a good idea to me.
struct btrfs_fs_info {
    uint8_t *tree_root;
    uint8_t *chunk_root;
    uint8_t *fs_root;
    uint8_t *extent_root;
};

// RB root item
//@todo: implement btrfs_root struct methods to hold RB roots
struct btrfs_root {
//    RB_ENTRY(btrfs_root) rb_node;

    struct btrfs_root_item root_item;
    struct btrfs_key root_key;
    struct btrfs_fs_info *fs_info;
};

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

    struct btrfs_superblock pm_superblock;      // superblock struct
    struct btrfs_fs_info pm_fsinfo;            // stores information on various rb roots


    LIST_HEAD(btrfs_sys_chunks, b_chunk_list) pm_backing_dev_bootstrap;

    struct lock pm_btrfslock;                   // protects allocations
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

#define VFSTOBTRFS(mp) ((struct btrfsmount_internal *)mp->mnt_data)
#define BTRFSLOGICALTOPHYSICAL(key, stripe, logical_addr) \
    (((struct btrfs_chunk_item_stripe *)stripe)->offset + \
    (logical_addr - ((struct btrfs_key *)key)->offset))

#endif // _BTRFS_MOUNT_H