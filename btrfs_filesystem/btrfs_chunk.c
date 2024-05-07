//
//  btrfs_chunk.c
//  macos-btrfs
//
//  Created by Yehia Hafez on 05/06/24.
//

#include <IOKit/IOLib.h>

#include "btrfs_chunk.h"
#include "btrfs_mapping.h"
#include "btrfs_header.h"
#include "btrfs_leaf.h"

#include <string.h>

/// bootstraps the chunk tree, extracting chunk_tree_record keypairs from the stripes
errno_t get_chunk_tree_bootstrap(superblock *sb_rec, btrfs_list **dest) {
	int off = 0;
	errno_t ret = 0;

	while(off < sb_rec->sys_chunk_array_valid) {
		// clearly there's nothing more to read, so we've got to break here
		if(off + sizeof(btrfs_key) > sb_rec->sys_chunk_array_valid) {
            DMESG_LOG("Short key read\n");
			break;
		} else if(off + sizeof(btrfs_chunk_item) > sb_rec->sys_chunk_array_valid) {
			DMESG_LOG("Short chunk read\n");
			break;
		}

		btrfs_key *key = (btrfs_key*)&sb_rec->sys_chunk_array[off];
		btrfs_chunk_item *chunk = (btrfs_chunk_item*)&sb_rec->sys_chunk_array[off + sizeof(btrfs_key)];

		if(key->obj_type != TYPE_CHUNK_ITEM) {
			DMESG_LOG("Unknown item type %d at offset %d\n", key->obj_type, off);
			break;
		}

		// if there are no stripes, something's not right
		if(chunk->num_stripes == 0) {
			DMESG_LOG("Found 0 stripes\n");
			break;
		}

		// if we have stripes, and the key object type is a chunk, there are no shortages
		// then the traversal can continue
		off += sizeof(btrfs_key) + sizeof(btrfs_chunk_item);

		for(int j = 0; j < chunk->num_stripes; ++j) {
            // we read a stripe
			btrfs_chunk_item_stripe *new_stripe = malloc(sizeof(btrfs_chunk_item_stripe));
            if(!new_stripe) {
                DMESG_LOG("Failed to allocate memory for a new stripe\n");
                return 0;   /// @todo: better error management
            }
			memcpy(new_stripe, &sb_rec->sys_chunk_array[off + (sizeof(btrfs_chunk_item_stripe) * j)], sizeof(btrfs_chunk_item_stripe));

            // we create a new chunk entry
            btrfs_chunk_tree_entry *new_chunk_entry = malloc(sizeof(btrfs_chunk_tree_entry));
            if(!new_chunk_entry) {
                DMESG_LOG("failed to allocate mem for a new_chunk_entry\n");
                IOFree(new_stripe, sizeof(btrfs_chunk_item_stripe));
                return 0;   /// @todo: better error management
            }

            memset(new_chunk_entry, 0, sizeof(btrfs_chunk_tree_entry));
            new_chunk_entry->key.size = chunk->size;
            new_chunk_entry->key.start = key->offset;
            new_chunk_entry->offset = new_stripe->offset;

            // we allocate a new node to be added to the list
			btrfs_list *new_node = malloc(sizeof(btrfs_list));
            if(!new_node) {
                DMESG_LOG("Failed to allocate mem for new_node\n");
                IOFree(new_stripe, sizeof(btrfs_chunk_item_stripe));
                IOFree(new_chunk_entry, sizeof(btrfs_chunk_tree_entry));
                return 0;
            }
			memset(new_node, 0, sizeof(btrfs_list));
			new_node->node = new_chunk_entry;

			ret += btrfs_list_push(new_node, dest);
            IOFree(new_stripe, sizeof(btrfs_chunk_item_stripe));   // free new_stripe after it's been copied and no longer needed
		}
		off += (sizeof(btrfs_chunk_item_stripe) * chunk->num_stripes);
	}
	return ret;
}

errno_t btrfs_read_chunk_tree(int fd, uint8_t *root, btrfs_list **dest_chunk_tree) {
	btrfs_tree_header header = btrfs_parse_header(root);
    errno_t ret = 0;

	if(header.level == 0) {
        btrfs_list *leaves = NULL;
		ret = btrfs_parse_leaf(root, &leaves);

        // traverse the leaves we just parsed
        for(btrfs_list *current = leaves->head; current != NULL; current = current->child) {
            btrfs_leaf_node *node = current->node;

            if(node->key.obj_type == TYPE_CHUNK_ITEM) {
                btrfs_chunk_item *chunk_item = (btrfs_chunk_item *) &root[sizeof(btrfs_tree_header) + node->offset];
                btrfs_chunk_item_stripe *chunk_stripe = (btrfs_chunk_item_stripe *) &root[sizeof(btrfs_tree_header) + sizeof(btrfs_chunk_item) + node->offset];

                btrfs_chunk_tree_entry *new_entry = malloc(sizeof(btrfs_chunk_tree_entry));
                if(!new_entry) {
                    DMESG_LOG("failed to allocate mem for a new_entry\n");
                    return 0;   /// @todo: better error management
                }
                new_entry->key.start = node->key.offset;
                new_entry->key.size = chunk_item->size;
                new_entry->offset = chunk_stripe->offset;

                // we allocate a new node to be added to the list
                btrfs_list *new_node = malloc(sizeof(btrfs_list));
                if(!new_node) {
                    DMESG_LOG("Failed to allocate mem for new_node\n");
                    IOFree(new_entry, sizeof(btrfs_chunk_tree_entry));
                    return 0;
                }
                memset(new_node, 0, sizeof(btrfs_list));
                new_node->node = new_entry;

                ret += btrfs_list_push(new_node, dest_chunk_tree);
            }
        }
        btrfs_list_destroy(&leaves, sizeof(btrfs_leaf_node));
	} else {
        DMESG_LOG("read_chunk_tree() stump- header level NOT 0\n");
    }
	return ret;
}
