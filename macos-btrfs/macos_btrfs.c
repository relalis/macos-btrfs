//
//  macos_btrfs.c
//  macos-btrfs
//
//  Created by Yehia Hafez on 5/23/21.
//

#include <mach/mach_types.h>
#include <libkern/libkern.h>
#include <libkern/locks.h>

#include "btrfs.h"



kern_return_t macos_btrfs_kext_start(kmod_info_t * ki, void *d);
kern_return_t macos_btrfs_kext_stop(kmod_info_t *ki, void *d);

kern_return_t macos_btrfs_kext_start(kmod_info_t * ki, void *d)
{
	return KERN_SUCCESS;
}

kern_return_t macos_btrfs_kext_stop(kmod_info_t *ki, void *d)
{
	return KERN_SUCCESS;
}

