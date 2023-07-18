//
//  btrfs_vfsops.c
//  macos-btrfs
//
//  Created by Yehia Hafez on 5/2/22.
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
#include "btrfs_parse.h"

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
	.vfs_mount = btrfs_vfs_mount,
	.vfs_unmount = btrfs_vfs_unmount,
	.vfs_init = btrfs_vfs_init,
	.vfs_start = btrfs_vfs_start,
	.vfs_root = btrfs_vfs_root,
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

	mount_t mp = vol->mp;
	vnode_t dev_vn = vol->dev_vn;
	buf_t primary_superblock;
	superblock *super = NULL;
	uint32_t blocksize = vfs_devblocksize(mp);

	errno_t err;

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

static errno_t btrfs_read_chunk_tree(btrfs_inmem_vol *vol, char *buffer) {
	btrfs_tree_header root_header = btrfs_parse_header(buffer);

	if(root_header.level == 0) {
		DMESG_LOG("btrfs_read_chunk_tree() parsing a leaf node");
		// We're parsing a leaf node here
		btrfs_leaf_list_record *items = btrfs_parse_leaf(buffer);
		btrfs_leaf_node_list_entry *cur_item;

		LIST_FOREACH(cur_item, items, ptrs) {
			DMESG_LOG("processing item of type %d", cur_item->node.key.obj_type);
			int test;
			if(cur_item->node.key.obj_type != TYPE_CHUNK_ITEM) continue;

			btrfs_chunk_item *nzItem = IOMallocZero(sizeof(btrfs_chunk_item));
			if(nzItem == NULL) return -ENOMEM;
			btrfs_chunk_item_stripe *nzStripe = IOMallocZero(sizeof(btrfs_chunk_item_stripe));
			if(nzStripe == NULL) return -ENOMEM;

			memcpy(nzItem,
				   buffer + sizeof(btrfs_tree_header) + cur_item->node.offset,
				   sizeof(btrfs_chunk_item)
			);
			
			memcpy(nzStripe,
				   buffer + sizeof(btrfs_tree_header) + cur_item->node.offset + sizeof(btrfs_chunk_item),
				   sizeof(btrfs_chunk_item_stripe)
			);
			
			DMESG_LOG("%llu", nzItem->stripe_length);

			btrfs_chunk_tree_entry *new_entry = IOMallocZero(sizeof(btrfs_chunk_tree_entry));
			if(new_entry == NULL) return -ENOMEM;

			new_entry->key.start = cur_item->node.key.offset;
			new_entry->key.size = nzItem->size;
			new_entry->value.offset = nzStripe->offset;

			DMESG_LOG("leaf item offset %llu size %llu stripe at %llu", cur_item->node.key.offset, nzItem->size, nzStripe->offset);
			LIST_INSERT_HEAD(vol->ct_rec, new_entry, ptrs);
		
			_FREE(nzItem, M_TEMP);
			_FREE(nzStripe, M_TEMP);
		}
	
		// @todo: this is a placeholder function, for now. We don't actually want to remove the items just yet.
		// Don't forget to free the list when you're done with it
		btrfs_leaf_node_list_entry *item, *tmp;
		LIST_FOREACH_SAFE(item, items, ptrs, tmp) {
			LIST_REMOVE(item, ptrs);
			_FREE(item, M_TEMP);
		}

	} else {
		DMESG_LOG("btrfs_read_chunk_tree() parsing an internal node");
		btrfs_internal_list_record *nodes = btrfs_parse_node(buffer);
		btrfs_internal_node_list_entry *cur_node;

		LIST_FOREACH(cur_node, nodes, ptrs) {
			DMESG_LOG("parse_node() stump");
		}

		btrfs_internal_node_list_entry *node, *tmp;
		LIST_FOREACH_SAFE(node, nodes, ptrs, tmp) {
			LIST_REMOVE(node, ptrs);
			_FREE(node, M_TEMP);
		}


		// We're parsing an internal node here
	}
	return 0;
}

static errno_t btrfs_bootstrap_chunk_tree(btrfs_inmem_vol *vol) {
	/// @discussion we only really need to read the root chunk item, and bootstrap from there
	/// the rest is just unnecessary. remove the loop, and build from there

	btrfs_chunk_item *t_chunk;
	btrfs_key *t_key;
	btrfs_chunk_item_stripe *t_stripe;
	btrfs_chunk_tree_entry *t_entry;

	vol->ct_rec = IOMallocZero(sizeof(btrfs_chunk_tree_record));
	if (vol->ct_rec == NULL) {
		return -ENOMEM; // or some appropriate error handling
	}
	// Initialize the list here
	LIST_INIT(vol->ct_rec);

	/// @todo: fix this

#warning this is generating a false chunk
	for(int i = 0; i < vol->sb_rec.sys_chunk_array_valid; i += (sizeof(btrfs_key) + sizeof(btrfs_chunk_item))) {
		if(i + sizeof(btrfs_key) > vol->sb_rec.sys_chunk_array_valid) { DMESG_LOG("short key read"); break; }

		t_key = (btrfs_key*)&vol->sb_rec.sys_chunk_array[i];

		i += sizeof(btrfs_key);

		t_chunk = (btrfs_chunk_item*)&vol->sb_rec.sys_chunk_array[i];
		
		i += sizeof(btrfs_chunk_item);

		if(t_key->obj_type != TYPE_CHUNK_ITEM) {
			DMESG_LOG("unknown item type %d in key at offset %d", t_key->obj_type, i);
			break;
		}
		
		DMESG_LOG("found %d stripes", t_chunk->num_stripes);
		if(t_chunk->num_stripes == 0) break;
		
		t_stripe = (btrfs_chunk_item_stripe*)&vol->sb_rec.sys_chunk_array[i];
		i += sizeof(btrfs_chunk_item_stripe) * t_chunk->num_stripes;

		t_entry = IOMallocZero(sizeof(btrfs_chunk_tree_entry));
		if(!t_entry) {
			return -ENOMEM;
		}
		t_entry->key.start = t_key->offset;
		t_entry->key.size = t_chunk->size;
		t_entry->value.offset = t_stripe->offset;
		DMESG_LOG("inserted key.start %llu key.size %llu value.offset %llu", t_key->obj_id, t_chunk->size, t_stripe->offset);
		LIST_INSERT_HEAD(vol->ct_rec, t_entry, ptrs);
	}
	return 0;
}

static errno_t btrfs_read_chunk_tree_root(btrfs_inmem_vol *vol, uint64_t chunk_root, kauth_cred_t cred) {
	btrfs_chunk_tree_entry *chunk = btrfs_get_chunk_from_logical_addr(vol, chunk_root);
	int error;

	if(chunk == NULL) {
		DMESG_LOG("failed on btrfs_get_chunk_from_logical_addr() call");
		return -EIO;
	}

	uint64_t offset = btrfs_chunk_get_offset(vol, chunk_root);
	if(offset == 0) {
		DMESG_LOG("failed to get root_tree offset.");
		return -EIO;
	}

	DMESG_LOG("chunk->key.size %llu phys offset %llu", chunk->key.size, offset);

	char *root = IOMallocZero(chunk->key.size);
	if(root == NULL) {
		DMESG_LOG("failed to allocate memory for chunk root");
		return -ENOMEM;
	}

	DMESG_LOG("physical root is located at offset %llu, blocknum %llu, size %llu", offset, offset / vol->sb_rec.sector_size, chunk->key.size);

	buf_t bp;

	// buf_meta_bread only reads up to 4k
	for(int i = 0; i < (chunk->key.size / vol->sb_rec.sector_size); ++i) {
		error = buf_meta_bread(vol->dev_vn, (offset / vol->sb_rec.sector_size) + i, vol->sb_rec.sector_size, cred, &bp);
		buf_setflags(bp, B_NOCACHE);
		
		if(error) {
			DMESG_LOG("failed to read chunk_tree_root block %llu", (offset / vol->sb_rec.sector_size) + i);
			goto error_exit;
		}

		memcpy(root + i * vol->sb_rec.sector_size, buf_dataptr(bp), vol->sb_rec.sector_size);
		buf_brelse(bp);
	}

	DMESG_LOG("read %llu bytes into chunk tree", chunk->key.size);

	error = btrfs_read_chunk_tree(vol, root);

	DMESG_LOG("btrfs_read_chunk_tree() result is %d", error);

error_exit:
	_FREE(root, M_TEMP);
	return error;
}

static errno_t btrfs_read_fs_tree_root(btrfs_inmem_vol *vol, char *root_buffer) {
	/// this should be the BTRFS root
	btrfs_tree_header root_header = btrfs_parse_header(root_buffer);

	errno_t ret = FSUR_IO_FAIL;

	if(root_header.level != 0) {
		DMESG_LOG("read_fs_tree_root() error - root tree must be a leaf node");
		return -EIO;
	}

	btrfs_leaf_list_record *items = btrfs_parse_leaf(root_buffer);
	btrfs_leaf_node_list_entry *cur_item;

	LIST_FOREACH(cur_item, items, ptrs) {
		if(cur_item->node.key.obj_id != BTRFS_ROOT_FSTREE || cur_item->node.key.obj_type != TYPE_ROOT_ITEM) continue;
		memcpy(&vol->fs_tree_root, root_buffer + cur_item->node.offset + sizeof(btrfs_tree_header), sizeof(btrfs_root_item));
		vol->fs_tree_root_off = btrfs_chunk_get_offset(vol, vol->fs_tree_root.block_number);
		ret = 0;
	}

	return ret;
}

static errno_t btrfs_read_root_tree_root(btrfs_inmem_vol *vol, uint64_t root_logical, kauth_cred_t cred) {
	btrfs_chunk_tree_entry *logical_record = btrfs_get_chunk_from_logical_addr(vol, root_logical);
	int error;

	if(logical_record == NULL) {
		DMESG_LOG("failed to retrieve btrfs_read_root_tree_root() logical record");
		return -EIO;
	}

	uint64_t offset = btrfs_chunk_get_offset(vol, root_logical);
	if(offset == 0) {
		DMESG_LOG("couldn't get physical offset for btrfs_read_root_tree_root()");
		return -EIO;
	}
	
	char *root_buffer = IOMallocZero(logical_record->key.size);
	if(root_buffer == NULL) {
		DMESG_LOG("failed to allocate mem for root_tree_root");
		return -ENOMEM;
	}

	buf_t bp;

	for(int i = 0; i < (logical_record->key.size / vol->sb_rec.sector_size); ++i) {
		error = buf_meta_bread(vol->dev_vn, (offset / vol->sb_rec.sector_size) + i, vol->sb_rec.sector_size, cred, &bp);
		buf_setflags(bp, B_NOCACHE);

		if(error) {
			DMESG_LOG("failed to read chunk_tree_root block %llu", (offset / vol->sb_rec.sector_size) + i);
			goto error_exit;
		}

		memcpy(root_buffer + i * vol->sb_rec.sector_size, buf_dataptr(bp), vol->sb_rec.sector_size);
		buf_brelse(bp);
	}

	error = btrfs_read_fs_tree_root(vol, root_buffer);
	if(error) {
		DMESG_LOG("failed to read fs_tree, errno %d", error);
		goto error_exit;
	}
error_exit:
	_FREE(root_buffer, M_TEMP);
	return error;
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

// frees the bootstrapped chunk tree
void free_chunk_tree(btrfs_chunk_tree_record* ct_rec) {
	if(ct_rec != NULL) {
		btrfs_chunk_tree_entry* entry;
		while (!LIST_EMPTY(ct_rec)) {
			entry = LIST_FIRST(ct_rec);
			LIST_REMOVE(entry, ptrs);
			_FREE(entry, M_TEMP);
		}
	}
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
	ret = btrfs_bootstrap_chunk_tree(vol);
	if(ret) {
		DMESG_LOG("failed to bootstrap chunk tree");
		goto error_exit;
	}

	ret = btrfs_read_chunk_tree_root(vol, vol->sb_rec.chunk_tree_addr, cred);
	if(ret) {
		DMESG_LOG("failed to read chunk tree root");
		goto error_exit;
	}

	ret = btrfs_read_root_tree_root(vol, vol->sb_rec.root_tree_addr, cred);
	if(ret) {
		DMESG_LOG("failed to read the root_tree_root");
		goto error_exit;
	}

	sfs->f_bsize = vol->sb_rec.sector_size;
	sfs->f_blocks = vol->sb_rec.total_bytes;
	sfs->f_bfree = vol->sb_rec.total_bytes - vol->sb_rec.bytes_used;
	sfs->f_iosize = vol->sb_rec.node_size;
	sfs->f_flags = vfs_flags(mp);
	strcpy(sfs->f_fstypename, "btrfs", 5);
	/*
    st->f_iosize = mntp->attr.f_iosize;
    st->f_bavail = mntp->attr.f_bavail;
    st->f_bused = mntp->attr.f_bused;
    st->f_files = mntp->attr.f_files;
    st->f_ffree = mntp->attr.f_ffree;
    st->f_fsid = mntp->attr.f_fsid;
    st->f_owner = mntp->attr.f_owner;*/

error_exit:
	if(vol && vol->ct_rec) free_chunk_tree(vol->ct_rec);
	_FREE(vol, M_TEMP);
	DMESG_LOG("btrfs_vfs_mount pre-mortem ret: %d", ret);

	/// this is to force the plugin to ALWAYS return input/output error. We only change this when it works, otherwise we're asking for the kernel to break.
	//ret = EIO;
	//btrfs_vfs_unmount(mp, MNT_FORCE, context);
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

// get root vnode of filesystem
static int btrfs_vfs_root(struct mount *mp, struct vnode **vpp, vfs_context_t context) {
	/// This results in an infinite loop, as no root is actually recovered.
	errno_t err = EAGAIN; // Couldn't find root dir

	vnode_t vn = NULL;
	struct vnode_fsparam param;

	kassert_nonnull(mp);
    kassert_nonnull(vpp);
    kassert_nonnull(context);

	DMESG_LOG("btrfs_vfs_root was called\n");
	btrfs_inmem_vol *vol = vfs_fsprivate(mp);
	kauth_cred_t cred = vfs_context_ucred(context);
	struct vnode *vp;

error_exit:
	return err;
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
