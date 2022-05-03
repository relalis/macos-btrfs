//
//  btrfs_volume.h
//  macos-btrfs
//
//  Created by Yehia Hafez on 5/2/22.
//

#ifndef btrfs_volume_h
#define btrfs_volume_h

#include <sys/mount.h>
#include <sys/types.h>
#include <libkern/OSAtomic.h>
#include <kern/locks.h>

typedef struct _btrfs_volume btrfs_volume;

#include "btrfs_filesystem.h"

#endif /* btrfs_volume_h */
