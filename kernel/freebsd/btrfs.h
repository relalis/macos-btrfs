#ifndef _BTRFS_TREE_OPS_H
#define _BTRFS_TREE_OPS_H

#include <sys/tree.h>
#include "btrfs_mount.h"

int btrfs_lookup_dir_item(struct btrfsmount_internal *bmp, struct btrfs_dir_item *dir_result, const char *name, int name_len);

struct b_chunk_list *bc_find_key_in_cache(struct btrfs_key key, struct btrfs_sys_chunks *head);
struct b_chunk_list *bc_find_logical_in_cache(uint64_t logical_addr, struct btrfs_sys_chunks *head);
uint64_t bc_logical_to_physical(struct btrfs_key search_key, uint64_t logical_addr, struct btrfs_sys_chunks *head);
int bc_add_to_chunk_cache(struct btrfs_key key, struct btrfs_chunk_item item, struct btrfs_chunk_item_stripe stripe, struct btrfs_sys_chunks *head);
void bc_free_cache_list(struct btrfs_sys_chunks *head);

#endif