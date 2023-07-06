//
//  btrfs_vnops.c
//  mount_btrfs
//
//  Created by Yehia Hafez on 4/6/23.
//

#include "btrfs_vnops.h"
#include "btrfs_volume.h"

// holds pointer to array of vnop functions for VFS plugin
int (**btrfs_vnop_p)(void *);

#define VNOP_FUNC   int (*)(void *)

static int btrfs_vnop_open(struct vnop_open_args *a);
static int btrfs_vnop_close(struct vnop_close_args *a);
static int btrfs_vnop_lookup(struct vnop_lookup_args *vfsp);
static int btrfs_vnop_getattr(struct vnop_getattr_args *);
static int btrfs_vnop_read(struct vnop_read_args *);
static int btrfs_vnop_write(struct vnop_write_args *);

static inline int btrfs_vnop_default(struct vnop_generic_args *arg)
{
	return ENOTSUP;
}


// Define the plugin's vnodeops struct
struct vnodeopv_entry_desc btrfs_vfs_vnodeop_entries[] = {
	{&vnop_default_desc, (VNOP_FUNC) btrfs_vnop_default},
	{ &vnop_open_desc, (VNOP_FUNC) btrfs_vnop_open },
	{ &vnop_close_desc, (VNOP_FUNC) btrfs_vnop_close },
	{ &vnop_getattr_desc, (VNOP_FUNC) btrfs_vnop_getattr },
	{ &vnop_read_desc, (VNOP_FUNC) btrfs_vnop_read },
	{ &vnop_write_desc, (VNOP_FUNC) btrfs_vnop_write },
	{ &vnop_lookup_desc, (VNOP_FUNC) btrfs_vnop_lookup },
	// Add any additional vnode operation functions here /
	// For example: { &vnop_create_desc, btrfs_vfs_create }, */
	{ NULL, NULL }
};

struct vnodeopv_desc btrfs_vfs_vnodeop_vector_desc = {
	&btrfs_vnop_p,
	btrfs_vfs_vnodeop_entries,
};

struct vnodeopv_desc *btrfs_vnopv_desc_list[1] = {
	&btrfs_vfs_vnodeop_vector_desc,
};


/* Define the plugin's vnode operation functions */

static int btrfs_vnop_open(struct vnop_open_args *a) {
	// Perform any open tasks here */
	return ENOTSUP;
}

static int btrfs_vnop_close(struct vnop_close_args *a) {
	// Perform any close tasks here
	return ENOTSUP;
}

static int btrfs_vnop_lookup(struct vnop_lookup_args *vfsp) {
	return ENOTSUP;
}

static int btrfs_vnop_getattr(struct vnop_getattr_args *ap    ) {
	// Perform any getattr tasks here
	return ENOTSUP;
}

static int btrfs_vnop_read(struct vnop_read_args *ap) {
	/*
	struct vnode *vp;
	struct uio *uio;
	btrfs_inmem_vol *vol;
	int error = 0;

	// Validate the arguments and vnode
	if (!ap || !ap->a_vp || ap->a_vp->v_type != VREG) {
		return EINVAL;
	}

	// Retrieve the vnode and associated volume
	vp = ap->a_vp;
	vol = vfs_fsprivate(vp->v_mount);
	if (!vol) {
		return EINVAL;
	}

	// Retrieve the uio struct for read operations
	uio = ap->a_uio;
	if (!uio) {
		return EINVAL;
	}

	// TODO: Locking considerations

	// Get the inode associated with the vnode
	btrfs_inode *inode = VTOI(vp);  // Convert vnode to btrfs_inode
	if (!inode) {
		error = ENOENT;  // Inode not found
		goto out;
	}

	// Handle read request
	while (uio_resid(uio) > 0) {
		// In BTRFS, files are stored in extents. An extent is a contiguous block of data.
		// The inode has a B-tree of extents (the "extent tree"), and each extent entry
		// has a key representing its offset in the file and a value representing the block
		// where it starts on disk and its length.

		// To read a file, you would locate the extent for the current file offset in the extent tree
		// and read from the block(s) it points to. If the requested read spans multiple extents,
		// you would have to handle that case as well.

		// For simplicity, let's assume you have a function read_extent() that reads from an extent
		// given an offset and length, and another function find_extent() that locates the extent
		// for a given file offset.
		uint64_t offset = uio_offset(uio);
		btrfs_extent *extent = find_extent(inode, offset);
		if (!extent) {
			error = EIO;  // Error reading extent
			goto out;
		}

		// Read data from extent
		size_t len = MIN(uio_resid(uio), extent->len);  // Don't read beyond the extent
		error = read_extent(vol, extent, offset, len, uio);
		if (error) {
			goto out;  // Error reading data
		}

		// Advance to the next extent if necessary
		if (uio_resid(uio) > 0) {
			// Advance uio_offset to the next extent (not shown here)
		}
	}

out:
	// TODO: Unlocking considerations

	return error;
	
	*/
	return ENOTSUP;
}

static int btrfs_vnop_write(struct vnop_write_args *a) {
// Perform any write tasks here
	return ENOTSUP;
}

// Define any additional vnode operation functions here
// For example: int btrfs_vfs_create(struct vnode *dvp, struct vnode **vpp, struct componentname *cnp, struct vnode_attr *vap) {...}
