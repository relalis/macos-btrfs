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

/*
* The original code is built off the MSDOSFS implementation in FreeBSD,
* found in sys/fs/msdosfs/msdosfs_vfsops.c. Everything beyond btrfs_mount()
* is, for the most part, original code based on research by Yehia Hafez.
* This software is provided "as is", and no requirement to include this comment
* block is added, but please respect Paul Popelka's original work and include the
* notice above this comment block.
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
#include "btrfs.h"
#include "btrfs_tree.h"

#ifdef LINUX_CROSS_BUILD
// clang on debian breaks fstack-protector as of 14.0.6
// this issue seems to be upstream at llvm, not debian
// for now, we're working around it
void *__stack_chk_guard = (void *)0xdeadbeef;
#endif

static const char btrfs_lock_msg[] = "btrfslk";

static const char *btrfs_mount_opts[] = {
        "ro", "uid", "gid", "from", NULL
};

static MALLOC_DEFINE(M_BTRFSMOUNT, "btrfs", "btrfs filesystem malloc");

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

//@todo: red-black trees
//RB_HEAD(btrfs_chunk_root, btrfs_root) b_chunk_root;

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

// called by the kernel when filesystem mount inits
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
        struct btrfs_superblock *prim_sblock;
        struct b_chunk_list *tmp_chunk_entry;
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
       // 
       // https://lists.freebsd.org/pipermail/freebsd-fs/2010-March/008011.html
       // 
        error = bread(devvp, (superblock_addrs[0] / BTRFS_SUPERBLOCK_SIZE) * 8, BTRFS_SUPERBLOCK_SIZE, NOCRED, &bp);
        if(error)
                goto error_exit; // fun times
        bp->b_flags |= B_AGE;
        prim_sblock = (struct btrfs_superblock *)bp->b_data;
        if(prim_sblock->magic != BTRFS_MAGIC) {
                error = EINVAL;
                goto error_exit;
        }

        bmp = malloc(sizeof(*bmp), M_BTRFSMOUNT, M_WAITOK | M_ZERO);
        bmp->pm_mountp = mp;
        bmp->pm_cp = cp;
        bmp->pm_bo = buf_obj;

        lockinit(&bmp->pm_btrfslock, 0, btrfs_lock_msg, 0, 0);

        // TASK_INIT for rw->ro

        // Initialize ownership to root assuming we're mounting disk as root
        // this module may never get to that point, but it's better to be safe
        // than leave open a gaping bug
        bmp->pm_uid = UID_ROOT;
        bmp->pm_gid = GID_WHEEL;
        bmp->pm_mask = bmp->pm_dirmask = S_IXUSR | S_IXGRP | S_IXOTH |
	    S_IRUSR | S_IRGRP | S_IROTH | S_IWUSR;

        // store superblock in the in-memory structure
        bmp->pm_superblock = *prim_sblock;

        bmp->pm_fsinfo.chunk_root = NULL;
        bmp->pm_fsinfo.tree_root = NULL;
        bmp->pm_fsinfo.fs_root = NULL;
        bmp->pm_fsinfo.extent_root = NULL;

        brelse(bp);

        LIST_INIT(&bmp->pm_backing_dev_bootstrap);

        for(int i = 0; i < bmp->pm_superblock.sys_chunk_array_valid; i += (sizeof(struct btrfs_key) + sizeof(struct btrfs_chunk_item))) {
                struct btrfs_key *fa_key = (struct btrfs_key *) &prim_sblock->sys_chunk_array[i];
		struct btrfs_chunk_item *fa_chunk = (struct btrfs_chunk_item *) &prim_sblock->sys_chunk_array[i + sizeof(struct btrfs_key)];
                struct btrfs_chunk_item_stripe *fa_stripe = (struct btrfs_chunk_item_stripe *) &prim_sblock->sys_chunk_array[i + sizeof(struct btrfs_key) + sizeof(struct btrfs_chunk_item)];
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
                } else if(fa_chunk->num_stripes > 1) {
                        // apparently the second stripe points to the next chunk_item
                        // btrfs documentation is somewhat.. lacking
                        uprintf("[BTRFS] Found %d stripes, but we're only processing 1\n", fa_chunk->num_stripes);
                }

                if(!bc_add_to_chunk_cache(*fa_key, *fa_chunk, *fa_stripe, &bmp->pm_backing_dev_bootstrap)) {
                        uprintf("[BTRFS] Duplicate chunk item %lu not added to cache\n", fa_stripe->offset);
                }

                // we continue the traversal and skip the stripes appended at the end of each element
                i += (sizeof(struct btrfs_chunk_item_stripe) * fa_chunk->num_stripes);
        }

        tmp_chunk_entry = bc_find_logical_in_cache(bmp->pm_superblock.chunk_tree_addr, &bmp->pm_backing_dev_bootstrap);

        // If we failed to bootstrap the chunk tree, we CANNOT continue
        if(tmp_chunk_entry == NULL)
                goto error_exit;

        // - Read the root tree root (requires chunk tree for logical->physical mapping)
        // - Read FS root to begin traversal
        bmp->pm_fsinfo.chunk_root = malloc(tmp_chunk_entry->chunk_item.size, M_BTRFSMOUNT, M_WAITOK | M_ZERO);

        error = bo_read_key_into_buf(devvp, tmp_chunk_entry->key, &bmp->pm_backing_dev_bootstrap, bmp->pm_fsinfo.chunk_root);
        if(error)
                goto error_exit;

        struct btrfs_tree_header head = BTRFSHEADER(bmp->pm_fsinfo.chunk_root);
        uint8_t *tst = BTRFSDATABUF(bmp->pm_fsinfo.chunk_root);
        for(int i = 0; i < head.num_items; ++i) {
                struct btrfs_leaf_node *to_dump_item = (struct btrfs_leaf_node *) (tst + (i * (sizeof(struct btrfs_leaf_node))));
                switch(to_dump_item->key.obj_type) {
                        case TYPE_CHUNK_ITEM: {
                                struct btrfs_chunk_item *tmp_chnk = (struct btrfs_chunk_item *)(tst + to_dump_item->offset);
                                for(int j = 0; j < tmp_chnk->num_stripes; ++j) {
                                        // We're skipping all chunks past the first.
                                        // Stripes after the first are for RAID setups, which we don't support
                                        if(j > 0)
                                                continue;
                                        struct btrfs_chunk_item_stripe *tmp_stripe = (struct btrfs_chunk_item_stripe *)(tst + to_dump_item->offset + sizeof(struct btrfs_chunk_item));
                                        if(!bc_add_to_chunk_cache(to_dump_item->key, *tmp_chnk, *tmp_stripe, &bmp->pm_backing_dev_bootstrap)) {
                                                uprintf("Overlap in chunk cache %lu %X\n", to_dump_item->key.obj_id, to_dump_item->key.obj_type);
                                        }
                                }
                                break;
                        }
                        default:
                                break;
                }
        }

        tmp_chunk_entry = bc_find_logical_in_cache(bmp->pm_superblock.root_tree_addr, &bmp->pm_backing_dev_bootstrap);
        // if we cannot read the root tree, we cannot continue
        if(tmp_chunk_entry == NULL)
                goto error_exit;

        bmp->pm_fsinfo.tree_root = malloc(tmp_chunk_entry->chunk_item.size, M_BTRFSMOUNT, M_WAITOK | M_ZERO);

        error = bo_read_key_into_buf(devvp, tmp_chunk_entry->key, &bmp->pm_backing_dev_bootstrap, bmp->pm_fsinfo.tree_root);
        if(error)
                goto error_exit;

        head = BTRFSHEADER(bmp->pm_fsinfo.tree_root);
        uprintf("TREE ROOT addr %lu num items %u level %d\n", head.address, head.num_items, head.level);

        // assign our internal structure to mp
        bmp->pm_devvp = devvp;
        bmp->pm_odevvp = odevvp;
        bmp->pm_dev = dev;

        mp->mnt_data = bmp;

        return(0);

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
                bc_free_cache_list(&bmp->pm_backing_dev_bootstrap);
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
        uprintf("[BTRFS] Unmount called\n");
        int error;
        struct btrfsmount_internal *bmp;

        error = 0;
        bmp = VFSTOBTRFS(mp);

        vn_lock(bmp->pm_devvp, LK_EXCLUSIVE | LK_RETRY);
        g_topology_lock();
        g_vfs_close(bmp->pm_cp);
        g_topology_unlock();
        BO_LOCK(&bmp->pm_odevvp->v_bufobj);
        bmp->pm_odevvp->v_bufobj.bo_flag &= ~BO_NOBUFS;
        BO_UNLOCK(&bmp->pm_odevvp->v_bufobj);
        atomic_store_rel_ptr((uintptr_t *)&bmp->pm_dev->si_mountpt, 0);
        mntfs_freevp(bmp->pm_devvp);
        vrele(bmp->pm_odevvp);
        dev_rel(bmp->pm_dev);

        bc_free_cache_list(&bmp->pm_backing_dev_bootstrap);
        if(bmp->pm_fsinfo.chunk_root != NULL)
                free(bmp->pm_fsinfo.chunk_root, M_BTRFSMOUNT);
        if(bmp->pm_fsinfo.tree_root != NULL)
                free(bmp->pm_fsinfo.tree_root, M_BTRFSMOUNT);

        lockdestroy(&bmp->pm_btrfslock);
        free(bmp, M_BTRFSMOUNT);
        mp->mnt_data = NULL;
        return(error);
}

static int btrfs_root(struct mount *mp, int flags, struct vnode **vpp) {
//        struct btrfsmount_internal *bmp = VFSTOBTRFS(mp);
//        return(bmp->pm_superblock.leaf_size);
        uprintf("[BTRFS] btrfs_root called\n");

        // After mount is called, this function is called. We're returning an error to kill the process
        // for testing purposes
        return(ENXIO);
}

static int btrfs_statfs(struct mount *mp, struct statfs *sbp) {
        return(0);
}

static int btrfs_fhtovp(struct mount *mp, struct fid *fhp, int flags, struct vnode **vpp) {
        return(0);
}

static struct vfsops btrfs_vfsops = {
	.vfs_fhtovp =		btrfs_fhtovp,
	.vfs_mount =		btrfs_mount,
	//.vfs_cmount =		btrfs_cmount,
	.vfs_root =		btrfs_root,
	.vfs_statfs =		btrfs_statfs,
	.vfs_unmount =		btrfs_unmount,
};

VFS_SET(btrfs_vfsops, btrfs, VFCF_READONLY );