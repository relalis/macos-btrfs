//
//  btrfs_vnops.c
//  mount_btrfs
//
//  Created by Yehia Hafez on 4/6/23.
//

#include "btrfs_vnops.h"

// holds pointer to array of vnop functions for VFS plugin
int (**btrfs_vnop_p)(void *);

#define VNOP_FUNC   int (*)(void *)

static int btrfs_vnop_open(struct vnop_open_args *a);
static int btrfs_vnop_close(struct vnop_close_args *a);
static int btrfs_vnop_lookup(struct vnop_lookup_args *vfsp);
static int btrfs_vnop_getattr(struct vnop_getattr_args *);
static int btrfs_vnop_read(struct vnop_read_args *);
static int btrfs_vnop_write(struct vnop_write_args *);

static inline int emptyfs_vnop_default(struct vnop_generic_args *arg)
{
	return ENOTSUP;
}


// Define the plugin's vnodeops struct
struct vnodeopv_entry_desc btrfs_vfs_vnodeop_entries[] = {
	{&vnop_default_desc, (VNOP_FUNC) emptyfs_vnop_default},
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

static int btrfs_vnop_read(struct vnop_read_args *a) {
	// Perform any read tasks here
	return ENOTSUP;
}

static int btrfs_vnop_write(struct vnop_write_args *a) {
// Perform any write tasks here
	return ENOTSUP;
}

// Define any additional vnode operation functions here
// For example: int btrfs_vfs_create(struct vnode *dvp, struct vnode **vpp, struct componentname *cnp, struct vnode_attr *vap) {...}
