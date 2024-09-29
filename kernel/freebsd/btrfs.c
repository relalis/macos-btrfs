#include <sys/systm.h>
#include <sys/mount.h>
#include <sys/queue.h>
#include <sys/param.h>
#include <sys/malloc.h>
#include <sys/kernel.h>
#include "btrfs.h"


static MALLOC_DEFINE(M_BTRFSOPS, "btrfs_ops", "btrfs operations malloc");

int btrfs_lookup_dir_item(struct btrfsmount_internal *bmp, struct btrfs_dir_item *dir_result, const char *name, int name_len) {
    return(0);
}

struct b_chunk_list *bc_find_key_in_cache(struct btrfs_key key, struct btrfs_sys_chunks *head) {
    struct b_chunk_list *cache_entry;

	LIST_FOREACH(cache_entry, head, entries) {
		if(cache_entry->key.obj_id == key.obj_id && cache_entry->key.obj_type == key.obj_type && (key.offset == 0 || cache_entry->key.offset == key.offset))
			return(cache_entry);
	}

	return NULL;
}

struct b_chunk_list *bc_find_logical_in_cache(uint64_t logical_addr, struct btrfs_sys_chunks *head) {
	struct b_chunk_list *cache_entry;
	
	LIST_FOREACH(cache_entry, head, entries) {
		if(logical_addr >= cache_entry->key.offset && logical_addr < (cache_entry->key.offset + cache_entry->chunk_item.size))
			return(cache_entry);
	}
	return NULL;
}

uint64_t bc_logical_to_physical(struct btrfs_key search_key, uint64_t logical_addr, struct btrfs_sys_chunks *head) {
	struct b_chunk_list *cache_entry;
	cache_entry = bc_find_key_in_cache(search_key, head);
	if(cache_entry)
		return(cache_entry->chunk_stripe.offset + (logical_addr - cache_entry->key.offset));
	return(0);
}

int bc_add_to_chunk_cache(struct btrfs_key key, struct btrfs_chunk_item item, struct btrfs_chunk_item_stripe stripe, struct btrfs_sys_chunks *head) {
	struct b_chunk_list *cache_entry;
	if(bc_find_key_in_cache(key, head) == NULL) {
		// this key isn't already cached in the chunk_tree list we hold
		cache_entry = malloc(sizeof(struct b_chunk_list), M_BTRFSOPS, M_WAITOK | M_ZERO);
		cache_entry->key = key;
		cache_entry->chunk_item = item;
		cache_entry->chunk_stripe = stripe;
		LIST_INSERT_HEAD(head, cache_entry, entries);
		return(1);
	}

	// there is a key in the chunk_tree list
	return(0);
}

void bc_free_cache_list(struct btrfs_sys_chunks *head) {
	if(!LIST_EMPTY(head)) {
		struct b_chunk_list *clr_np;
		while(!LIST_EMPTY(head)) {
			clr_np = LIST_FIRST(head);
			LIST_REMOVE(clr_np, entries);
			free(clr_np, M_BTRFSOPS);
			clr_np = NULL;
		}
	}
}