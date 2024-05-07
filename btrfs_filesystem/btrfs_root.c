#ifndef __APPLE__
#define _XOPEN_SOURCE 500
#endif

#include <IOKit/IOLib.h>
#include <unistd.h>
#include <string.h>

#include "btrfs_root.h"
#include "btrfs_mapping.h"
#include "btrfs_header.h"
#include "btrfs_leaf.h"

uint8_t *btrfs_read_root(int fd, uint64_t chunk_root, btrfs_list *chunk_bootstrap) {
    btrfs_chunk_tree_entry *entry = btrfs_read_logical(chunk_bootstrap, chunk_root);
    uint64_t physical_offset = btrfs_chunk_get_offset(chunk_bootstrap, chunk_root);

    if(entry == NULL) {
        DMESG_LOG("btrfs_read_root() failed to read a chunk tree entry at root %lu\n", chunk_root);
        return NULL;
    }

    if(entry->key.size <= 0) {
        DMESG_LOG("btrfs_read_root() key size is =< 0, returning\n");
        return NULL;
    }

    uint8_t *root = malloc(entry->key.size);
    if(root == NULL) {
        DMESG_LOG("failed to allocate mem for btrfs_read_root() offset %lu\n", chunk_root);
        return NULL;
    }

    memset(root, 0, entry->key.size);

    pread(fd, root, entry->key.size, physical_offset);

    DMESG_LOG("read_root() logical offset %lu, physical offset %lu, size %lu\n", chunk_root, physical_offset, entry->key.size);

    return root;
}

uint8_t *read_fs_tree_root(int fd, superblock *sb_rec, uint8_t *root, btrfs_list *ct_rec) {
    errno_t err;
    uint8_t *ret = NULL;

    btrfs_tree_header header = btrfs_parse_header(root);

    DMESG_LOG("root tree root level=%u, bytenr=%lu, nritems=%u\n",
                header.level, header.address, header.num_items);

    if(header.level != 0) {
        DMESG_LOG("Root tree root is not a leaf node.\n");
        return NULL;
    }

    btrfs_list *leaves = NULL;
    err = btrfs_parse_leaf(root, &leaves);
    if(!err) {
        DMESG_LOG("Failed to parse leaves\n");
        return NULL;
    }


    for(btrfs_list *current = leaves->head; current != NULL; current = current->child) {
        btrfs_leaf_node *node = current->node;
        if(node->key.obj_id != BTRFS_ROOT_FSTREE || node->key.obj_type != TYPE_ROOT_ITEM) {
            // OH MY GOD. ACTUALLY GO TO NEXT.
            // THE WHOLE POINT WAS THIS. CHANTING THE CURRENT NODE
            // DOESN'T ADVANCE THE LOOP FURTHER. THIS WAS THE BUG. FUCK.
            continue;
        }

        btrfs_root_item *root_item = malloc(sizeof(btrfs_root_item));
        if(!root_item) {
            DMESG_LOG("Failed to allocate mem for root_item\n");
            return NULL;
        }
        memset(root_item, 0, sizeof(btrfs_root_item));

        memcpy(root_item, root + sizeof(btrfs_tree_header) + node->offset, sizeof(btrfs_root_item));

        uint64_t physical_addr = btrfs_chunk_get_offset(ct_rec, root_item->block_number);

        ret = malloc(sb_rec->node_size);
        if(!ret) {
            DMESG_LOG("Failed to allocate memory for fs_tree_root item!\n");
            IOFree(root_item, sizeof(btrfs_root_item));
            goto func_exit;
        } else {
            pread(fd, ret, sb_rec->node_size, physical_addr/*-65536*/);
            DMESG_LOG("fs tree root at logical offset=%lu, physical offset=%lu, size=%u\n",
                    root_item->block_number, physical_addr, sb_rec->node_size);
            IOFree(root_item, sizeof(btrfs_root_item));
            break;
        }
    }

func_exit:
    btrfs_list_destroy(&leaves, sizeof(btrfs_leaf_node));

    return ret;
}
