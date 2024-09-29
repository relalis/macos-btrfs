#ifndef _RW_OPS_H
#define _RW_OPS_H

#include <unistd.h>

/// @details: we *MIGHT* use a btrfs_read_from_fd() helper function because different kernels/userspace libs
/// use different read methods. On macOS, this is buf_meta_bread(), which reads in chunks. If we're calling
/// from userspace, the basic pread() in unistd is more than enough.
#ifdef __linux__
#define btrfs_read_from_fd(w, x, y, z) pread(w, x, y, z)
#elif __APPLE__
#error btrfs_read_from_fd() has not been defined yet
#endif

#endif