#include <IOKit/IOLib.h>
#include "btrfs_filesystem.h"

btrfs_tree_header btrfs_parse_header(char *buffer) {
    btrfs_tree_header this;
    memcpy(&this, buffer, sizeof(btrfs_tree_header));
    return this;
}