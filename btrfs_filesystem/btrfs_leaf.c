//
//  btrfs_leaf.c
//  macos-btrfs
//
//  Created by Yehia Hafez on 03/03/24.
//

#include <IOKit/IOLib.h>
#include "btrfs_leaf.h"
#include "btrfs_header.h"

/// @todo: instead of returning -ENOMEM, destroy the list first then return -ENOMEM
/// add a goto for funsies
errno_t btrfs_parse_leaf(uint8_t *buffer, btrfs_list **dest) {
    // caller MUST call btrfs_list_destroy on the resulting list
    errno_t ret = 0;

    // parse header
    btrfs_tree_header header = btrfs_parse_header(buffer);

    for(int i = 0; i < header.num_items; ++i) {
        // initialize a new leaf
        btrfs_leaf_node *new_leaf = IOMallocZero(sizeof(btrfs_leaf_node));
        if(new_leaf == NULL) {
            DMESG_LOG("failed to allocate mem for leaf %d", i);
            return -ENOMEM;
        }

        // copy leaf sized data from the buffer to the new leaf
        memcpy(new_leaf, buffer + sizeof(btrfs_tree_header) + (i * sizeof(btrfs_leaf_node)), sizeof(btrfs_leaf_node));
        // create a new node to be inserted
        btrfs_list *insert = (btrfs_list *)IOMallocZero(sizeof(btrfs_list));

        // the new leaf is the payload of the new node
        insert->node = new_leaf;
        // push the new leaf to the leaf stack
        ret = btrfs_list_push(insert, dest);
        if(!btrfs_list_push(insert, dest)){
            DMESG_LOG("failed to push leaf %d", i);
            IOFree(insert, sizeof(btrfs_list));
            IOFree(new_leaf, sizeof(btrfs_leaf_node));
            return -ENOMEM;
        }
    }
    return ret;
}

errno_t btrfs_parse_node(uint8_t *buffer, btrfs_list **dest) {
    // caller MUST call btrfs_list_destroy on the resulting list
    errno_t ret = 0;

    // parse header
    btrfs_tree_header header = btrfs_parse_header(buffer);

    int offset = sizeof(btrfs_tree_header);

    for(int i = 0; i < header.num_items; ++i) {
        // initialize a new node
        btrfs_internal_node *new_node = IOMallocZero(sizeof(btrfs_internal_node));
        if(new_node == NULL) {
            DMESG_LOG("failed to allocate mem for node %d", i);
            return -ENOMEM;
        }

        // copy node sized data from the buffer to the new node

        memcpy(new_node, buffer + offset, sizeof(btrfs_internal_node));

        // create a new node to be inserted
        btrfs_list *insert = IOMallocZero(sizeof(btrfs_list));

        // the new internal node is the payload of the new insert node
        insert->node = new_node;
        // push the new node to the node stack
        ret = btrfs_list_push(insert, dest);
        if(!ret){
            DMESG_LOG("failed to push node %d", i);
            IOFree(new_node, sizeof(btrfs_internal_node));
            IOFree(insert, sizeof(btrfs_list));
            return -ENOMEM;
        }
        offset += sizeof(btrfs_internal_node);
    }
    return ret;
}