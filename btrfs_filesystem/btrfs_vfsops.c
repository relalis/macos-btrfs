//
//  btrfs_vfsops.c
//  macos-btrfs
//
//  Created by Yehia Hafez on 5/2/22.
//

#include <libkern/libkern.h>

#include <sys/disk.h>
#include <sys/fcntl.h>
#include <sys/kernel_types.h>
#include <sys/mount.h>
#include <sys/systm.h>
#include <sys/stat.h>
#include <sys/vnode.h>
#include <mach/kmod.h>

#include <libkern/OSKextLib.h>
#include <IOKit/IOLib.h>

#include "btrfs_volume.h"
#include "btrfs_vnops.h"

/// Driver-wide lock to protect global data structures
lck_grp_t *btrfs_lock = NULL;

static int btrfs_vfs_init(struct vfsconf *vfsp);
static int btrfs_vfs_start(struct mount *, int, vfs_context_t);
static int btrfs_vfs_mount(struct mount *, vnode_t, user_addr_t data, vfs_context_t);
static int btrfs_vfs_unmount(struct mount *, int, vfs_context_t);
static int btrfs_vfs_root(struct mount *, struct vnode **, vfs_context_t);
static int btrfs_vfs_getattr(struct mount *, struct vfs_attr *, vfs_context_t);

#define DMESG_LOG(fmt, ...) printf("[macos-BTRFS]: " fmt "\n", ##__VA_ARGS__);
/*
 * Define the plugin's vfsops struct
 * stores function pointers to all VFS routines
 * functions DO NOT operate on fs vnodes, but on filesystem instances
 */
struct vfsops btrfs_vfs_vfsops = {
	.vfs_mount = btrfs_vfs_mount,
	.vfs_unmount = btrfs_vfs_unmount,
	.vfs_init = btrfs_vfs_init,
	.vfs_start = btrfs_vfs_start,
	.vfs_root = btrfs_vfs_root,
	.vfs_getattr = btrfs_vfs_getattr
};

#define BTRFS_NAME "BTRFS"

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

static errno_t btrfs_read_superblock_record(btrfs_inmem_vol *vol, kauth_cred_t cred, buf_t *buf) {
	mount_t mp = vol->mp;
	vnode_t dev_vn = vol->dev_vn;
	buf_t primary_superblock;
	superblock *super = NULL;
	uint32_t blocksize = vfs_devblocksize(mp);

	errno_t err = EIO;

	DMESG_LOG("stepped into btrfs_read_superblock_record()");

	err = buf_meta_bread(dev_vn, superblock_addrs[0]/BTRFS_SUPERBLOCK_SIZE, blocksize, cred, &primary_superblock);

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


/* Define the plugin's initialization function */
int btrfs_vfs_init(struct vfsconf *vfsp) {
	// Perform any initialization tasks here */
	return 0;
}

// VFS calls this to indicate mount is ready for usage
// apparently this is unnecessary
static int btrfs_vfs_start(struct mount *mp, int flags, vfs_context_t context) {
	return 0;
}

// Define the plugin's mount function
int btrfs_vfs_mount(struct mount *mp, vnode_t devvp, user_addr_t data, vfs_context_t context) {
	int ret = 0;
	daddr64_t nr_blocks;
	btrfs_inmem_vol *vol;
	kauth_cred_t cred;
	buf_t tempBuffer;
	dev_t dev;
	superblock *sb_rec;
	struct vfsstatfs *sfs = vfs_statfs(mp);
	uint32_t blocksize;

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
	sfs->f_fsid.val[0] = (int32_t) vfs_typenum(mp);

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

	tempBuffer = NULL;
	sb_rec = NULL;

	ret = btrfs_read_superblock_record(vol, cred, &tempBuffer);
	DMESG_LOG("post rec, superblock magic: %llu", vol->sb_rec.magic);

error_exit:
	DMESG_LOG("btrfs_vfs_mount pre-mortem ret: %d", ret);
	btrfs_vfs_unmount(mp, MNT_FORCE, context);
	return ret;
}

// Define the plugin's unmount function
int btrfs_vfs_unmount(struct mount *mp, int mntflags, vfs_context_t context) {
	// Perform any unmount tasks here
	OSKextReleaseKextWithLoadTag(OSKextGetCurrentLoadTag());
	DMESG_LOG("unloaded btrfs_vfs_unmount()");
	return 0;
}

// get root vnode of filesystem
static int btrfs_vfs_root(struct mount *mp, struct vnode **vpp, vfs_context_t context) {
	return 0;
}

// get information about this filesystem
static int btrfs_vfs_getattr(struct mount *mp, struct vfs_attr *vpp, vfs_context_t context) {
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
