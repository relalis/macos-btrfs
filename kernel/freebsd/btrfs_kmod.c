/*-
 * SPDX-License-Identifier: BSD-4-Clause
 *
 * Copyright (C) 1994, 1995, 1997 Wolfgang Solfrank.
 * Copyright (C) 1994, 1995, 1997 TooLs GmbH.
 * All rights reserved.
 * Original code by Paul Popelka (paulp@uts.amdahl.com) (see below).
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by TooLs GmbH.
 * 4. The name of TooLs GmbH may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY TOOLS GMBH ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL TOOLS GMBH BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/*-
 * Written by Paul Popelka (paulp@uts.amdahl.com)
 *
 * You can do anything you want with this software, just don't say you wrote
 * it, and don't remove this notice.
 *
 * This software is provided "as is".
 *
 * The author supplies this software to be publicly redistributed on the
 * understanding that the author is not responsible for the correct
 * functioning of this software in any circumstances and is not liable for
 * any damages caused by this software.
 *
 * October 1992
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/buf.h>
#include <sys/module.h>
#include <sys/conf.h>
#include <sys/fcntl.h>
#include <sys/kernel.h>
#include <sys/vnode.h>
#include <sys/mount.h>
#include <sys/namei.h>
#include <sys/priv.h>
#include <sys/proc.h>
#include <sys/rwlock.h>
#include <sys/stat.h>
#include <sys/vnode.h>

#include <geom/geom.h>
#include <geom/geom_vfs.h>

#include "btrfs_mount.h"

static const char btrfs_lock_msg[] = "btrfslk";

static const char *btrfs_mount_opts[] = {
        "ro", "uid", "gid", NULL
};

static MALLOC_DEFINE(M_BTRFSMOUNT, "btrfs", "btrfs filesystem");

// update the mount point
static int update_mp(struct mount *mp, struct thread *td);
static int mount_btrfs_filesystem(struct vnode *devvp, struct mount *mp);
// since this will be RO, this method is somewhat redundant. will keep anyway
static void btrfs_remount_ro(void *arg, int pending);
// file handler to vnode ptr
static vfs_fhtovp_t btrfs_fhtovp;
static vfs_mount_t btrfs_mount;
static vfs_root_t btrfs_root;
static vfs_statfs_t btrfs_statfs;
static vfs_sync_t btrfs_sync;
static vfs_unmount_t btrfs_unmount;

static int update_mp(struct mount *mp, struct thread *td) {
        struct btrfsmount_internal *bmp = (struct btrfsmount_internal *)mp->mnt_data;

        int v;

        if(vfs_scanopt(mp->mnt_optnew, "gid", "%d", &v) == 1)
                bmp->pm_gid = v;
        if(vfs_scanopt(mp->mnt_optnew, "uid", "%d", &v) == 1)
                bmp->pm_uid = v;
        if(vfs_scanopt(mp->mnt_optnew, "mask", "%d", &v) == 1)
                bmp->pm_mask = v & ALLPERMS;
        if(vfs_scanopt(mp->mnt_optnew, "dirmask", "%d", &v) == 1)
                bmp->pm_dirmask = v & ALLPERMS;

        return(0);
}

// @todo: understand cmount better
static int btrfs_cmount(struct mntarg *mnt_args, void *data, uint64_t flags) {
        struct btrfs_args args;
        int error;

        if(data == NULL)
                return(EINVAL);
        error = copyin(data, &args, sizeof args);

        if(error)
                return(error);
        
        mnt_args = mount_argsu(mnt_args, "from", args.fspec, MAXPATHLEN);
        mnt_args = mount_arg(mnt_args, "export", &args.export, sizeof(args.export));
        mnt_args = mount_argf(mnt_args, "uid", "%d", args.uid);
        mnt_args = mount_argf(mnt_args, "gid", "%d", args.gid);
        mnt_args = mount_argf(mnt_args, "mask", "%d", args.mask);
        mnt_args = mount_argf(mnt_args, "dirmask", "%d", args.dirmask);

        error = kernel_mount(mnt_args, flags);

        return (error);
}

/// @brief called by the kernel when filesystem mount inits
/// @param mp 
/// @return 
static int btrfs_mount(struct mount *mp) {
        struct vnode *devvp;
        struct thread *td;

        struct btrfsmount_internal *bmp = NULL;
        struct nameidata ndp;
        int error;
        accmode_t accmode;      // access permissions
        char *from;

        td = curthread;

        if(vfs_filteropt(mp->mnt_optnew, btrfs_mount_opts))
                return(EINVAL);

        /*
                @todo: read/write checks, update_mp logic
        */

       // We're either not updating the mp, or we're updating the name. In this case,
       // look up name and verify it refers to sensible disk device.
       if(vfs_getopt(mp->mnt_optnew, "from", (void **)&from, NULL))
                return(EINVAL);

        // NDINIT initializes nameidata structure
        NDINIT(&ndp, LOOKUP, FOLLOW | LOCKLEAF, UIO_SYSSPACE, from);
        error = namei(&ndp);
        if(error)
                return(error);
        devvp = ndp.ni_vp; // result vnode
        NDFREE_PNBUF(&ndp); // free

        if(!vn_isdisk_error(devvp, &error)) {
                vput(devvp);
                return(error);
        }


        // if non root user mounting, verify permissions
        accmode = VREAD;
        if((mp->mnt_flag & MNT_RDONLY) == 0)
                accmode |= VWRITE;
        error = VOP_ACCESS(devvp, accmode, td->td_ucred, td);
        if(error)
                error = priv_check(td, PRIV_VFS_MOUNT_PERM); // sys/priv.h
        if(error) {
                vput(devvp);
                return(error);
        }
        if((mp->mnt_flag & MNT_UPDATE) == 0) {
                // update complete, start mounting
                error = mount_btrfs_filesystem(devvp, mp);
        } else {
                // @todo: understand why msdosfs does this.
                vput(devvp);
                if(devvp != bmp->pm_odevvp)
                        return(EINVAL);
        }

        if(error) {
                vrele(devvp);
                return(error);
        }

        error = update_mp(mp, td);
        if(error) { 
                if((mp->mnt_flag & MNT_UPDATE) == 0)
                        btrfs_unmount(mp, MNT_FORCE);
                return(error);
        }

        vfs_mountedfrom(mp, from);
        return(0);
}

static int mount_btrfs_filesystem(struct vnode *odevvp, struct mount *mp) {
        struct btrfsmount_internal *bmp;
        struct buf *bp;
        struct cdev *dev;
	struct vnode *devvp;
        btrfs_superblock *prim_sblock;
        struct bufobj *buf_obj;

        int ronly, error;
        struct g_consumer *cp;

        bp = NULL;
        bmp = NULL;

        ronly = (mp->mnt_flag & MNT_RDONLY) != 0;

        devvp = mntfs_allocvp(mp, odevvp);
        dev = devvp->v_rdev;
        if(atomic_cmpset_acq_ptr((uintptr_t *)&dev->si_mountpt, 0, (uintptr_t)mp) == 0) {
                mntfs_freevp(devvp);
                return(EBUSY);
        }

        // sleep queue lock
        g_topology_lock();
        error = g_vfs_open(devvp, &cp, "btrfs", ronly ? 0 : 1);
        g_topology_unlock();

        if(error != 0) {
                atomic_store_rel_ptr((uintptr_t *)&dev->si_mountpt, 0);
                mntfs_freevp(devvp);
                return(error);
        }
        dev_ref(dev);
        buf_obj = &devvp->v_bufobj;
        BO_LOCK(&odevvp->v_bufobj); // sys/rwlock.h
        VOP_UNLOCK(devvp);

        if(dev->si_iosize_max != 0)
                mp->mnt_iosize_max = dev->si_iosize_max;
        if(mp->mnt_iosize_max > maxphys)
                mp->mnt_iosize_max = maxphys;

        /*
                READ SUPERBLOCK
                CHECK MAGIC
        */
        error = bread(devvp, superblock_addrs[0] / BTRFS_SUPERBLOCK_SIZE, BTRFS_SUPERBLOCK_SIZE, NOCRED, &bp);
        if(error)
                goto error_exit; // fun times
        bp->b_flags |= B_AGE;
        prim_sblock = (btrfs_superblock *)bp->b_data;
        if(prim_sblock->magic != BTRFS_MAGIC) {
                error = EINVAL;
                goto error_exit;
        }

        bmp = malloc(sizeof(*bmp), M_BTRFSMOUNT, M_WAITOK | M_ZERO);
        bmp->pm_mountp = mp;
        bmp->pm_cp = cp;
        bmp->pm_bo = buf_obj;

        lockinit(&bmp->pm_btrfslock, 0, btrfs_lock_msg, 0, 0);
        lockinit(&bmp->pm_checkpath_lock, 0, "btrfs_chkpath", 0, 0);

        // TASK_INIT for rw->ro

        // Initialize ownership to root assuming we're mounting disk as root
        // this module may never get to that point, but it's better to be safe
        // than leave open a gaping bug
        bmp->pm_uid = UID_ROOT;
        bmp->pm_gid = GID_WHEEL;
        bmp->pm_mask = bmp->pm_dirmask = S_IXUSR | S_IXGRP | S_IXOTH |
	    S_IRUSR | S_IRGRP | S_IROTH | S_IWUSR;

        // @todo: should we copy the entire superblock to memory, or do we just need
        // some references?
        // the sector size is certainly important for future block reads, but the 
        // btrfs superblock has a chunk tree array that we don't really need beyond
        // bootstrapping (afaik) if we're gonna write.
        // on the other hand, holding on to that array could be useful if we misread
        // something, or if there's data corruption. We'll cross that bridge when
        // we get to it.

        bmp->pm_sector_size = prim_sblock->sector_size;
        bmp->pm_superblock_physical_addr = prim_sblock->sb_phys_addr;
        bmp->pm_root_addr = prim_sblock->root_tree_addr;

        // read sys_chunk_array to bootstrap the system chunk tree
        RB_INIT(&bmp->pm_chunk_bootstrap);
        for(int i = 0; i < prim_sblock->sys_chunk_array_valid; i += (sizeof(btrfs_key) + sizeof(btrfs_chunk_item))) {
                btrfs_key *fa_key = (btrfs_key *)&prim_sblock->sys_chunk_array[i];
		btrfs_chunk_item *fa_chunk = (btrfs_chunk_item *)&prim_sblock->sys_chunk_array[i + sizeof(btrfs_key)];

                // the chunk tree is essential. If we encounter an error, we'll
                // simply exit in error.
                if(fa_key->obj_type != TYPE_CHUNK_ITEM) {
                        error = EINVAL;
                        goto error_exit;
                }
                // every chunk has a stripe. absence of a stripe is corrupt data.
                if(fa_chunk->num_stripes == 0) {
                        error = EINVAL;
                        goto error_exit;
                }

                struct b_chunk_bstrap *n_node = malloc(sizeof(struct b_chunk_bstrap), M_BTRFSMOUNT, M_WAITOK | M_ZERO);
                memcpy(n_node, &prim_sblock->sys_chunk_array[i], sizeof(btrfs_key) + sizeof(btrfs_chunk_item));

                n_node->stripe_head = malloc(sizeof(struct chunk_stripe_list), M_BTRFSMOUNT, M_WAITOK | M_ZERO);
                LIST_INIT(n_node->stripe_head);
                for(int j = 0; j < fa_chunk->num_stripes; ++j) {
                        struct b_stripe_list *n_stripe = malloc(sizeof(struct b_stripe_list), M_BTRFSMOUNT, M_WAITOK | M_ZERO);
                        memcpy(&n_stripe->val, &prim_sblock->sys_chunk_array[i]
                                        + (sizeof(btrfs_key) + sizeof(btrfs_chunk_item))
                                        + (sizeof(btrfs_chunk_item_stripe) * j),
                                sizeof(btrfs_chunk_item_stripe));
                        LIST_INSERT_HEAD(n_node->stripe_head, n_stripe, entries);
                        i += sizeof(btrfs_chunk_item_stripe);
                }

                RB_INSERT(sys_chunk_map, &bmp->pm_chunk_bootstrap, n_node);
        }

        // in order for traversal:

        // - Read the chunk tree root
        // - Read the root tree root (requires chunk tree for logical->physical mapping)
        // - Read the filesystem tree root (will be found under the root tree)

        // release the buffer
        brelse(bp);
        bp = NULL;

error_exit:
        if(bp != NULL)
                brelse(bp);
        if(cp != NULL) {
                g_topology_lock();
                g_vfs_close(cp);
                g_topology_unlock();
        }
        if(bmp != NULL) {
                lockdestroy(&bmp->pm_btrfslock);
                lockdestroy(&bmp->pm_checkpath_lock);
                //free(bmp->)
                mp->mnt_data = NULL;
        }
        BO_LOCK(&odevvp->v_bufobj);
        odevvp->v_bufobj.bo_flag &= ~BO_NOBUFS;
        BO_UNLOCK(&odevvp->v_bufobj);
        atomic_store_rel_ptr((uintptr_t *)&dev->si_mountpt, 0);
        vn_lock(devvp, LK_EXCLUSIVE | LK_RETRY);
        mntfs_freevp(devvp);
        dev_rel(dev);
        return(error);
}

static int btrfs_unmount(struct mount *mp, int mntflags) {
        int error = 0;

        return(error);
}

// THIS IS NOT SUPPOSED TO BE HERE
// I'm just keeping this so I can build in peace
// @todo remove this
static int event_handler(struct module *module, int event, void *arg) {

        int e = 0; /* Error, 0 for normal return status */

        switch (event) {

        case MOD_LOAD:

                uprintf("BTRFS Filesystem module loaded\n");

                break;

        case MOD_UNLOAD:

                uprintf("BTRFS Filesystem module unloaded\n");

                break;

        default:

                e = EOPNOTSUPP; /* Error, Operation Not Supported */

                break;

        }


        return(e);

}

/* The second argument of DECLARE_MODULE. */

static moduledata_t btrfs_mod_conf = {

    "btrfs",    /* module name */

     event_handler,  /* event handler */

     NULL            /* extra data */

};

/*
        - init sequence:

        btrfs_props_init
        btrfs_init_sysfs
        btrfs_init_compress
        btrfs_init_cachep
        btrfs_transaction_init
        btrfs_ctree_init
        btrfs_free_space_init
        extent_state_init_cachep
        extent_buffer_init_cachep
        btrfs_bioset_init
        extent_map_init
        ordered_data_init
        btrfs_delayed_inode_init
        btrfs_auto_defrag_init
        btrfs_delayed_ref_init
        btrfs_prelim_ref_init
        btrfs_interface_init
        btrfs_print_mod_info
        btrfs_run_sanity_tests
        register_btrfs
*/

static int init_btrfs_fs(void) {
        int ret = 0;

        return ret;
}

DECLARE_MODULE(btrfs_fsm, btrfs_mod_conf, SI_SUB_DRIVERS, SI_ORDER_MIDDLE);
