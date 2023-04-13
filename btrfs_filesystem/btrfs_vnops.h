//
//  btrfs_vnops.h
//  macos-btrfs
//
//  Created by Yehia Hafez on 4/7/23.
//

#ifndef btrfs_vnops_h
#define btrfs_vnops_h

#include <sys/mount.h>
#include <sys/vnode.h>

extern struct vnodeopv_entry_desc btrfs_vfs_vnodeop_entries[];
extern struct vnodeopv_desc *btrfs_vnopv_desc_list[1];

extern int (**btrfs_vnop_p)(void *);

#endif /* btrfs_vnops_h */
