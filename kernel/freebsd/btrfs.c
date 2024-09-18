#include "btrfs.h"

static int btrfs_comp_keys_by_type(const struct btrfs_key *k1, const struct btrfs_key *k2) {
    if((k1->obj_id > k2->obj_id) || (k1->obj_type > k2->obj_type) || (k1->offset > k2->offset))
        return 1;
    if((k1->obj_id < k2->obj_id) || (k1->obj_type < k2->obj_type) || (k1->offset < k2->offset))
        return -1;
    return 0;
}