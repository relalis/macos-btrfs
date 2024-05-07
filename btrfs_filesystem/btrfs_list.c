#include "btrfs_volume.h"
#include <IOKit/IOLib.h>


errno_t btrfs_list_push(btrfs_list *src, btrfs_list **dest) {
    if (src == NULL) {
        return 0; // node to be appended needs to be allocated by caller func
    }

    if (*dest == NULL || (*dest)->head == NULL) {
        *dest = src;
        src->head = src;
        src->tail = src;
    } else {
        // If the list is not empty, append src to the end of the list
        (*dest)->tail->child = src; // Set the current tail's child to src
        src->parent = (*dest)->tail;
        src->head = (*dest)->head;
        (*dest)->head->tail = src; // Update the head's tail pointer to src
        src->tail = src; // Ensure src's tail pointer points to itself, being the last element
    }

    return 1;
}

errno_t btrfs_list_destroy(btrfs_list **list, size_t node_size) {
    if (list == NULL || *list == NULL) {
        return 0; // Error handling for invalid input
    }

    btrfs_list *current = (*list)->head;
    while (current != NULL) {
        btrfs_list *next = current->child;

        // If the node contains dynamically allocated data, free it here
        if (current->node != NULL) {
            IOFree(current->node, node_size); // Free the node's data
            current->node = NULL; // Avoid dangling pointer
        }

        IOFree(current, sizeof(btrfs_list)); // Free the current list node
        current = next; // Move to the next node
    }

    // After all nodes have been freed, reset the original list pointer to NULL
    *list = NULL;

    return 1; // Success
}