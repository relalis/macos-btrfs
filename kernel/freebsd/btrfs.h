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