//
//  macos_btrfs.c
//  macos-btrfs
//
//  Created by Yehia Hafez on 5/23/21.
//

#include <sys/mount.h>

#include <mach/mach_types.h>
#include <mach/kmod.h>
#include <mach/machine/vm_param.h>

#include <libkern/libkern.h>
#include <libkern/OSMalloc.h>
#include <libkern/locks.h>

#include "btrfs.h"

/// Driver-wide lock to protect global data structures
static lck_mtx_t *btrfs_lock;

/*!
 @function btrfs_mount mount a btrfs filesystem
 @param mp mount point to initialize/mount
 @param dev_vn vnode of the device we are mounting
 @param data mount options (in user space)
 @param context vfs context
 @abstract The VFS calls this via VFS_MOUNT() when it wants to mount a BTRFS volume.
 @discussion Return 0 on success and errno on error
 We call OSKextRetainKextWithLoadTag() to prevent the KEXT from being
 unloaded automatically while in use.  If the mount fails, we must call
 OSKextReleaseKextWithLoadTag() to allow the KEXT to be unloaded.
 */
static int btrfs_mount(mount_t mp, vnode_t dev_vn, user_addr_t data, vfs_context_t context) {
	struct vfsstatfs *sfs = vfs_statfs(mp);
	return 0;
}
static int btrfs_unmount(mount_t mp, int mnt_flags, vfs_context_t context __unused);
static int btrfs_root(mount_t mp, struct vnode **vpp, vfs_context_t context __unused);
static int btrfs_getattr(mount_t mp, struct vfs_attr *fsa, vfs_context_t context __unused);
static int btrfs_sync(struct mount *mp, int waitfor, vfs_context_t context __attribute__((unused)));
static int btrfs_vget(mount_t mp, ino64_t ino, struct vnode **vpp, vfs_context_t context __unused);
static int btrfs_setattr(struct mount *mp, struct vfs_attr *fsa, vfs_context_t context);

static struct vfsops btrfs_vfsops = {
	.vfs_mount	= btrfs_mount,
	.vfs_unmount	= btrfs_unmount,
	.vfs_root	= btrfs_root,
	.vfs_getattr	= btrfs_getattr,
	.vfs_sync	= btrfs_sync,
	.vfs_vget	= btrfs_vget,
	.vfs_setattr	= btrfs_setattr,
};

/// Lock groups and malloc tags
static lck_grp_attr_t *btrfs_lock_grp_attr;
lck_grp_t *btrfs_lock_grp;
lck_attr_t *btrfs_lock_attr;
OSMallocTag btrfs_malloc_tag;

static vfstable_t btrfs_vfstable;

kern_return_t macos_btrfs_kext_start(kmod_info_t * ki __unused, void *d __unused);
kern_return_t macos_btrfs_kext_stop(kmod_info_t * ki __unused, void *d __unused);

kern_return_t macos_btrfs_kext_start(kmod_info_t * ki __unused, void *d __unused)
{
	struct vfs_fsentry vfe;

	if (btrfs_lock_grp_attr || btrfs_lock_grp || btrfs_lock_attr || btrfs_malloc_tag)
		panic("%s(): Lock(s) and/or malloc tag already initialized.\n", __FUNCTION__);
	
	btrfs_lock_grp_attr = lck_grp_attr_alloc_init();
	if (!btrfs_lock_grp_attr) {
		printf("BTRFS: Failed to allocate a lock element.\n");
		return KERN_MEMORY_FAILURE;
	}
	
	btrfs_lock_grp = lck_grp_alloc_init("com.relalis.btrfs", btrfs_lock_grp_attr);
	if(!btrfs_lock_grp) {
		printf("BTRFS: Failed to allocate a lock element.\n");
		return KERN_MEMORY_FAILURE;
	}
	
	btrfs_lock_attr = lck_attr_alloc_init();
	if(!btrfs_lock_attr) {
		printf("BTRFS: Failed to allocate a lock element.\n");
		return KERN_MEMORY_FAILURE;
	}
	
	btrfs_malloc_tag = OSMalloc_Tagalloc("com.relalis.btrfs", OSMT_DEFAULT);
	if(!btrfs_malloc_tag) {
		printf("BTRFS: OSMalloc_Tagalloc() failed.\n");
		return KERN_MEMORY_FAILURE;
	}
	
	/// Initialize driver-wide lock
	lck_mtx_init(&btrfs_lock, btrfs_lock_grp, btrfs_lock_attr);

//	vfe = (struct vfs_fsentry) {
//		.vfe_vfsops	=
	return KERN_SUCCESS;
}

kern_return_t macos_btrfs_kext_stop(kmod_info_t * ki __unused, void *d __unused)
{
	return KERN_SUCCESS;
}

