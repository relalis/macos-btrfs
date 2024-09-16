#include "btrfs.h"
#include "crc32.h"
#include "rw_ops.h"

#include <errno.h>


static inline uint64_t btrfs_name_hash(const char *name, int len) {
	return crc32(name, len);
}

/// @todo proper castagnolli crc32 checksumming
uint64_t btrfs_get_crc32_checksum(uint8_t *buffer, int len) {
    uint64_t crc = crc32(NULL, 0);
    for(int i = 0; i < len; ++i) {
        crc = crc32(buffer + i, i);
    }
    return crc;
}

errno_t btrfs_get_volume_superblock_record(int mount_dev_fd, btrfs_superblock *result_superblock) {
	/// @todo: there can be more superblocks, implement them later
	/// @todo: add a check for device length

    // the caller MUST allocate btrfs_superblock-sized memory in the destination
    if(result_superblock == NULL)
        return -ENOMEM;

    if(btrfs_read_from_fd(mount_dev_fd, result_superblock, BTRFS_SUPERBLOCK_SIZE, superblock_addrs[0]) != BTRFS_SUPERBLOCK_SIZE)
        return FSUR_IO_FAIL;

    if(result_superblock->magic == BTRFS_MAGIC)
        /// @todo verify checksum
        //if(btrfs_get_crc32_checksum(result_superblock, sizeof(btrfs_superblock)) == result_superblock->checksum)
            return FSUR_RECOGNIZED;

    // we shouldn't reach here.
    return FSUR_UNRECOGNIZED;
}