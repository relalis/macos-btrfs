//
//  btrfs_vfsops.c
//  macos-btrfs
//
//  Created by Yehia Hafez on 05/02/22.
//


#include <sys/kernel_types.h>
#include <sys/types.h>
#include <vfs/vfs_support.h>
#include <sys/disk.h>
#include <sys/fcntl.h>
#include <sys/mount.h>
#include <sys/systm.h>
#include <sys/stat.h>
#include <sys/vnode.h>
#include <mach/kmod.h>

#include <libkern/libkern.h>

#include <libkern/OSKextLib.h>
#include <IOKit/IOLib.h>

#include <sys/loadable_fs.h>

#include "btrfs_volume.h"
#include "btrfs_vnops.h"
#include "btrfs_mapping.h"
#include "btrfs_header.h"
#include "btrfs_leaf.h"
#include "btrfs_chunk.h"
#include "btrfs_root.h"

/// Driver-wide lock to protect global data structures
lck_grp_t *btrfs_lock = NULL;

static int btrfs_vfs_init(struct vfsconf *vfsp);
static int btrfs_vfs_start(struct mount *, int, vfs_context_t);
static int btrfs_vfs_mount(struct mount *, vnode_t, user_addr_t data, vfs_context_t);
static int btrfs_vfs_unmount(struct mount *, int, vfs_context_t);
static int btrfs_vfs_root(struct mount *, struct vnode **, vfs_context_t);
static int btrfs_vfs_getattr(struct mount *, struct vfs_attr *, vfs_context_t);

/*
 * Define the plugin's vfsops struct
 * stores function pointers to all VFS routines
 * functions DO NOT operate on fs vnodes, but on filesystem instances
 */
struct vfsops btrfs_vfs_vfsops = {
	/// function to mount a file system on a particular vnode. It is responsible for initializing data structures,
	/// and filling in the vfs structure with all the relevant information (such as the vfs_data field). 
	.vfs_mount = btrfs_vfs_mount,
	/// function to release this file system, or unmount it. It is the one, for example, responsible for detecting
	/// that a file system has still opened resources that cannot be released, and for returning an errno code that
	/// results in the user process getting a ``device busy'' error. 
	.vfs_unmount = btrfs_vfs_unmount,

	.vfs_init = btrfs_vfs_init,
	.vfs_start = btrfs_vfs_start,
	/// will return the root vnode of this file system. Each file system has a root vnode from which traversal to all
	/// other vnodes in the file system is enabled. This vnode usually is hand crafted (via kernel_malloc) and not
	/// created as part of the standard ways of creating new vnodes (i.e. vn_lookup). 
	.vfs_root = btrfs_vfs_root,
	/// get attributes of file or dir???
	.vfs_getattr = btrfs_vfs_getattr
};

#define BTRFS_NAME "btrfs"

#define ARRAY_SIZE(x)       (sizeof(x) / sizeof(*(x)))
#define BTRFS_VFS_FLAGS ( \
	VFS_TBLTHREADSAFE   |   \
	VFS_TBLFSNODELOCK   |   \
	VFS_TBLNOTYPENUM    |   \
	VFS_TBLLOCALVOL     |   \
	VFS_TBL64BITREADY   |   \
	0                       \
)

static struct vfs_fsentry btrfs_vfsentry = {
	&btrfs_vfs_vfsops,
	ARRAY_SIZE(btrfs_vnopv_desc_list),
	btrfs_vnopv_desc_list,
	0,
	BTRFS_NAME,
	BTRFS_VFS_FLAGS,
	{NULL, NULL},
};

/* Define the plugin's vfsconf struct */
struct vfsconf btrfs_vfs_conf = {
	.vfc_typenum = -1,
	.vfc_refcount = 0,
	.vfc_name = "BTRFS",
	.vfc_flags = 0,
};

btrfs_inmem_vol *btrfs_get_mount_from_mp(mount_t __nonnull mp) {
	btrfs_inmem_vol *btrfsvol;
	kassert_nonnull(mp);
	btrfsvol = vfs_fsprivate(mp);
	kassert_nonnull(btrfsvol);
	kassert(btrfsvol->sb_rec.magic == BTRFS_MAGIC);
	kassert(btrfsvol->mp == mp);
	return btrfsvol;
}

static errno_t btrfs_blocksize_set(mount_t mp, vnode_t dev_vn, uint32_t bsize, vfs_context_t ctx) {
	errno_t err;
	struct vfsioattr ia;

	err = VNOP_IOCTL(dev_vn, DKIOCSETBLOCKSIZE, (caddr_t)&bsize, FWRITE, ctx);
	if(err) return ENXIO;

	/// Update the cached block size in the mount point, i.e. the value returned by vfs_devblocksize()
	vfs_ioattr(mp, &ia);
	ia.io_devblocksize = bsize;
	vfs_setioattr(mp, &ia);

	/// Update the block size in the block device, i.e. the v_specsize of the device vnode.
	set_fsblocksize(dev_vn);
	return 0;
}

static errno_t btrfs_read_superblock_record(btrfs_inmem_vol *vol, kauth_cred_t cred) {
	/// @discussion BTRFS stores at least 3 superblocks, indicated by `superblock_addrs[]`, and writes to all 3 simultaneously. for the time being, we're only checking the first (as mounting fails if the first is corrupted anyway, and later will be adding checks for the remaining superblocks.

	buf_t primary_superblock;
	superblock *super = NULL;
	uint32_t blocksize = vfs_devblocksize(vol->mp);

	errno_t err;

	DMESG_LOG("stepped into btrfs_read_superblock_record()");

	err = buf_meta_bread(vol->dev_vn, superblock_addrs[0]/BTRFS_SUPERBLOCK_SIZE, blocksize, cred, &primary_superblock);

	buf_setflags(primary_superblock, B_NOCACHE);

	if(!err) {
		super = (superblock *)buf_dataptr(primary_superblock);
		if(err) {
			DMESG_LOG("failed to map buffer of primary_superblock (error %d)", err);
			super = NULL;
		} else {
			if(super->magic == BTRFS_MAGIC) {
				memcpy(&vol->sb_rec, super, sizeof(superblock));
			}
			buf_brelse(primary_superblock);
			return 0;
		}
		DMESG_LOG("superblock is invalid");
		err = EIO;
	} else {
		DMESG_LOG("there was an error in btrfs_read_superblock_record() %d", err);
		super = NULL;
	}
	buf_brelse(primary_superblock);
	return err;
}

/*static errno_t btrfs_read_chunk_tree(btrfs_inmem_vol *vol, char *buffer) {
	btrfs_tree_header root_header = btrfs_parse_header(buffer);

	if(root_header.level == 0) {
		DMESG_LOG("btrfs_read_chunk_tree() parsing a leaf node");
		btrfs_list *items;

		if(btrfs_parse_leaf(buffer, items)) {
			// traverse along the list
			while(items->child != 0) {
				// create a new temporary item to hold the node
				btrfs_leaf_node *cur_item = items->node;
				if(cur_item->key.obj_type != TYPE_CHUNK_ITEM) {
					items = items->child;
					continue;
				}

				btrfs_chunk_item *new_chunk = IOMallocZero(sizeof(btrfs_chunk_item));
				if(new_chunk == NULL) {
					return -ENOMEM; // or some proper memory handling
				}
				btrfs_chunk_item_stripe *new_stripe = IOMalloc(sizeof(btrfs_chunk_item_stripe));
				if(new_stripe == NULL) {
					return -ENOMEM;
				}
			}
		}
	}
}*/

/* Define the plugin's initialization function */
int btrfs_vfs_init(struct vfsconf *vfsp) {
	// Perform any initialization tasks here */
	return 0;
}

// VFS calls this to indicate mount is ready for usage
// apparently this is unnecessary
static int btrfs_vfs_start(struct mount *mp, int flags, vfs_context_t context) {
	return 1;
}

/// @todo fixme
static int btrfs_get_root_vnode(btrfs_inmem_vol * __nonnull btrfsmp, vnode_t * __nonnull vpp) {
	int err = EAGAIN, err2;
	vnode_t vn = NULL;
	uint32_t vid;
	struct vnode_fsparam param;

	kassert_nonnull(btrfsmp);
	kassert_nonnull(vpp);
	kassert_null(*vpp);

	lck_mtx_lock(btrfsmp->mtx_root);

	while(err = EAGAIN) {
		kassert_null(vn);
		lck_mtx_assert(btrfsmp->mtx_root, LCK_MTX_ASSERT_OWNED);

		if(btrfsmp->is_root_attaching) {
			btrfsmp->is_root_waiting = true;
			(void) msleep(&btrfsmp->rootvp, btrfsmp->mtx_root, PINOD, NULL, NULL);
			kassert(btrfsmp->is_root_waiting == false);
			err = EAGAIN;
		} else if(btrfsmp->rootvp == NULLVP) {
			// no root vnode detected
			btrfsmp->is_root_attaching = true;
			lck_mtx_unlock(btrfsmp->mtx_root);

			param.vnfs_mp = btrfsmp->mp;
			param.vnfs_vtype = VDIR;
			param.vnfs_str = "btrfs";
			param.vnfs_dvp = NULL;
			param.vnfs_fsnode = NULL;
			param.vnfs_vops = btrfs_vnop_p;
			param.vnfs_markroot = 1;
			param.vnfs_marksystem = 0;

			param.vnfs_rdev = 0; // VBLK / VCHR support
			param.vnfs_filesize = 0;	// VDIR doesn't have filesize
			param.vnfs_cnp = NULL;
			param.vnfs_flags = VNFS_NOCACHE | VNFS_CANTCACHE;
			
			err = vnode_create(VNCREATE_FLAVOR, sizeof(param), &param, &vn);
			if(err == 0) {
				kassert_nonnull(vn);
				DMESG_LOG("vnode_create() ok vn: %p vid: %#x", vn, vnode_vid(vn));
			} else {
				kassert_null(vn);
				DMESG_LOG("vnode_create() fail errno: %d", err);
			}

			lck_mtx_lock(btrfsmp->mtx_root);
			if(err == 0) {
				kassert_null(btrfsmp->rootvp);
				btrfsmp->rootvp = vn;
				err2 = vnode_addfsref(vn);
				kassertf(err2 == 0, "vnode_addfsref() fail errno: %d", err2);

				kassert(btrfsmp->is_root_attaching);
				btrfsmp->is_root_attaching = false;
				if(btrfsmp->is_root_waiting) {
					btrfsmp->is_root_waiting = false;
					wakeup(btrfsmp->rootvp);
				}
			}
		} else {
			// root vnode exists, obtain vnode vid
			vn = btrfsmp->rootvp;
			kassert_nonnull(vn);
			vid = vnode_vid(vn);
			lck_mtx_unlock(btrfsmp->mtx_root);

			err = vnode_getwithvid(vn, vid);
			if(err == 0) { }
			else {
				DMESG_LOG("vnode_getwithvid() fail errno: %d", err);
				vn = NULL;
				err = EAGAIN;
			}
			lck_mtx_lock(btrfsmp->mtx_root);
		}
	}

	lck_mtx_unlock(btrfsmp->mtx_root);
	if(err == 0) {
		kassert_nonnull(vn);
		*vpp = vn;
	} else kassert_null(vn);

	return err;
}

// Define the plugin's mount function
int btrfs_vfs_mount(struct mount *mp, vnode_t devvp, user_addr_t data, vfs_context_t context) {
	int ret = 0;
	daddr64_t nr_blocks;
	btrfs_inmem_vol *vol;
	kauth_cred_t cred;
	dev_t dev;
	superblock *sb_rec;
	struct vfsstatfs *sfs = vfs_statfs(mp);
	int blocksize;

	DMESG_LOG("entering btrfs_vfs_mount()");
	OSKextRetainKextWithLoadTag(OSKextGetCurrentLoadTag());

	cred = vfs_context_proc(context) ? vfs_context_ucred(context) : NOCRED;

	/// read-only for now
	vfs_setflags(mp, MNT_RDONLY);

	/// as this is a real mount request, dev cannot be NULL
	dev = vnode_specrdev(devvp);

	/// vfs advisory locking
	vfs_setlocklocal(mp);

	sfs->f_fsid.val[0] = (int32_t) dev;
//	sfs->f_fsid.val[0] = (int32_t) vfs_typenum(mp);

	dev = vnode_specrdev(devvp);

	vol = IOMallocZero(sizeof(btrfs_inmem_vol));
	if(!vol) {
		DMESG_LOG("failed to allocate mem");
		ret = ENOMEM;
		goto error_exit;
	} else {
		*vol = (btrfs_inmem_vol) {
			.mp = mp,
			.dev = dev,
			.dev_vn = devvp,
		};
	}

	blocksize = vfs_devblocksize(mp);

	/// BTRFS by design is a 64-bit filesystem, so we need to change the blocksize to 4096 bytes
	if(blocksize < PAGE_SIZE_64) {
		ret = btrfs_blocksize_set(mp, devvp, PAGE_SIZE_64, context);
		if(ret) {
			DMESG_LOG("Failed to set device block size to 4096 because DKIOCSETBLOCKSIZE ioctl returned error %d", ret);
			goto error_exit;
		}
		blocksize = PAGE_SIZE_64;
	} else {
		DMESG_LOG("blocksize %u is greater than or equal to PAGE_SIZE_64", blocksize);
	}

	DMESG_LOG("blocksize is %d, PAGE_SIZE_64 is %llu", blocksize, PAGE_SIZE_64);

	/// Get the size of the device in units of blocksize bytes
	ret = VNOP_IOCTL(devvp, DKIOCGETBLOCKCOUNT, (caddr_t) &nr_blocks, 0, context);
	if(ret) {
		DMESG_LOG("failed to determine device size (ioctl error %d)", ret);
		ret = ENXIO;
		goto error_exit;
	}

	vol->num_blocks = nr_blocks;

	sb_rec = NULL;

	ret = btrfs_read_superblock_record(vol, cred);
	if(ret) {
		DMESG_LOG("not a BTRFS volume");
		goto error_exit;
	}

	DMESG_LOG("post rec, superblock magic: %llu", vol->sb_rec.magic);
	vfs_setfsprivate(mp, vol);

	sfs->f_bsize = vol->sb_rec.sector_size;
	sfs->f_blocks = vol->sb_rec.total_bytes;
	sfs->f_bfree = vol->sb_rec.total_bytes - vol->sb_rec.bytes_used;
	sfs->f_iosize = vol->sb_rec.node_size;
	sfs->f_flags = vfs_flags(mp);

	/*
    st->f_iosize = mntp->attr.f_iosize;
    st->f_bavail = mntp->attr.f_bavail;
    st->f_bused = mntp->attr.f_bused;
    st->f_files = mntp->attr.f_files;
    st->f_ffree = mntp->attr.f_ffree;
    st->f_fsid = mntp->attr.f_fsid;
    st->f_owner = mntp->attr.f_owner;*/


	strcpy(sfs->f_fstypename, "btrfs", 5);


	btrfs_list *chunk_tree_bootstrap = NULL;

	ret = get_chunk_tree_bootstrap(sb_rec, &chunk_tree_bootstrap);

	if(!ret) {
		DMESG_LOG("Failed to bootstrap chunk tree");
		goto error_exit;
	}

//#error "don't forget to replace io_device with buf_meta_bread dev_vn, dummy"

//	err = buf_meta_bread(vol->dev_vn, superblock_addrs[0]/BTRFS_SUPERBLOCK_SIZE, blocksize, cred, &primary_superblock);

	uint8_t *chunk_tree_root = btrfs_read_root(vol->dev_vn, sb_rec->chunk_tree_addr, chunk_tree_bootstrap); // chunk root

	if(!chunk_tree_root) {
		DMESG_LOG("Failed to read chunk tree root");
		goto error_exit;
	}

	ret = btrfs_read_chunk_tree(vol->dev_vn, chunk_tree_root, &chunk_tree_bootstrap);
	
	if(!ret) {
		DMESG_LOG("failed to parse chunk tree");
		goto error_exit;
	}

	uint8_t *root_tree_root = btrfs_read_root(vol->dev_vn, sb_rec->root_tree_addr, chunk_tree_bootstrap);
	if(!root_tree_root) {
		DMESG_LOG("failed to read root_tree_root");
		goto error_exit;
	}

	uint8_t *fs_tree_root = read_fs_tree_root(vol->dev_vn, sb_rec, root_tree_root, chunk_tree_bootstrap);
	if(!fs_tree_root) {
		DMESG_LOG("Failed to read the fs tree root");
		goto error_exit;
	}

error_exit:
	if(vol && vol->ct_rec) _FREE(vol->ct_rec, M_TEMP);
	_FREE(vol, M_TEMP);
	DMESG_LOG("btrfs_vfs_mount pre-mortem ret: %d", ret);

	/// this is to force the plugin to ALWAYS return input/output error. We only change this when it works, otherwise we're asking for the kernel to break.
	ret = EIO;
	btrfs_vfs_unmount(mp, MNT_FORCE, context);
	return ret;
}

// Define the plugin's unmount function
int btrfs_vfs_unmount(struct mount *mp, int mntflags, vfs_context_t context) {
	btrfs_inmem_vol *vol;
	vol = vfs_fsprivate(mp);
	if(!vol) goto unload_exit;
	// Perform any unmount tasks here

unload_exit:
	OSKextReleaseKextWithLoadTag(OSKextGetCurrentLoadTag());
	DMESG_LOG("unloaded btrfs_vfs_unmount()");
	return 0;
}

static vfstable_t btrfs_vfstbl_ref = NULL;

/* Initialize and finalize the plugin */
kern_return_t macos_btrfs_start(kmod_info_t *ki, void *data) {
	DMESG_LOG("macos_btrfs_start()");
	kern_return_t e;
	e = vfs_fsadd(&btrfs_vfsentry, &btrfs_vfstbl_ref);
	DMESG_LOG("vfs_add() done");
	if(e != 0) {
		DMESG_LOG("vfs_fsadd() failure errno: %d", e);
		e = KERN_FAILURE;
	}

	return e;
}

kern_return_t macos_btrfs_stop(kmod_info_t *ki, void *data) {
	DMESG_LOG("kext stop now");
	kern_return_t e;
	e = vfs_fsremove(btrfs_vfstbl_ref);
	if(e != 0) {
		DMESG_LOG("vfs_fsremove() failure errno: %d", e);
		e = KERN_FAILURE;
	}

	return e;
}

#ifdef __kext_makefile__
extern kern_return_t _start(kmod_info_t *, void *);
extern kern_return_t _stop(kmod_info_t *, void *);

/* will expand name if it's a macro */
#define KMOD_EXPLICIT_DECL2(name, ver, start, stop) \
    __attribute__((visibility("default")))          \
        KMOD_EXPLICIT_DECL(name, ver, start, stop)

KMOD_EXPLICIT_DECL2(BUNDLEID, KEXTBUILD_S, _start, _stop)

/* if you intended to write a kext library  NULLify _realmain and _antimain */
__private_extern__ kmod_start_func_t *_realmain = macos_btrfs_start;
__private_extern__ kmod_stop_func_t *_antimain = macos_btrfs_stop;

__private_extern__ int _kext_apple_cc = __APPLE_CC__;
#endif
