/*
Copyright (c) 2024, Yehia Hafez 
All rights reserved. 

Redistribution and use in source and binary forms, with or without 
modification, are permitted provided that the following conditions are met: 

 * Redistributions of source code must retain the above copyright notice, 
   this list of conditions and the following disclaimer. 
 * Redistributions in binary form must reproduce the above copyright 
   notice, this list of conditions and the following disclaimer in the 
   documentation and/or other materials provided with the distribution. 

THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND ANY 
EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE FOR ANY 
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT 
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY 
OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH 
DAMAGE. 
*/

#include <sys/systm.h>
#include <sys/mount.h>
#include <sys/queue.h>
#include <sys/param.h>
#include <sys/malloc.h>
#include <sys/kernel.h>
#include <sys/buf.h>
#include "btrfs.h"


static MALLOC_DEFINE(M_BTRFSOPS, "btrfs_ops", "btrfs operations malloc");

int btrfs_lookup_dir_item(struct btrfsmount_internal *bmp, struct btrfs_dir_item *dir_result, const char *name, int name_len) {
    return(0);
}

int bo_read_key_into_buf(struct vnode *devvp, struct btrfs_key key, struct btrfs_sys_chunks *cache_head, uint8_t *dest) {
        // tmp_chunk_entry->key
        struct buf *bp;
        int error = 1, offset = 0;
        struct b_chunk_list *chunk_entry;
        uint64_t phys_addr;

        if(dest == NULL) {
                uprintf("[BTRFS] bad buffer passed to btrfs_read_key_into_buf()\n");
                goto read_fail;
        }
        chunk_entry = bc_find_logical_in_cache(key.offset, cache_head);
        if(!chunk_entry) {
                uprintf("[BTRFS] Failed to find a chunk tree cache entry for %lu\n", key.offset);
                goto read_fail;
        }

        phys_addr = bc_logical_to_physical(chunk_entry->key, key.offset, cache_head);

        int bytes_remaining = chunk_entry->chunk_item.size;
        while(bytes_remaining > 0) {
                size_t bytes_to_read = (bytes_remaining > MAXBCACHEBUF) ? MAXBCACHEBUF : bytes_remaining;
                daddr_t block_num = (phys_addr + offset) / DEV_BSIZE; // phys addr to blocknr
                size_t block_offset = (phys_addr + offset) % DEV_BSIZE; // offset in blocknr

                error = bread(devvp, block_num, bytes_to_read + block_offset, NOCRED, &bp);
                if (error != 0) {
                        brelse(bp);
                        goto read_fail;
                }

                memcpy(dest + offset, bp->b_data + block_offset, bytes_to_read);
                brelse(bp);

                offset += bytes_to_read;
                bytes_remaining -= bytes_to_read;
        }
        return(bytes_remaining); // should be zero

read_fail:
        return(error);
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