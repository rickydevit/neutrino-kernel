#include "list.h"
#include <liballoc.h>
#include <stdbool.h>

// === PRIVATE FUNCTIONS ========================

void _node_impl_init(ListNode* self) {
    self->lock = NewLock;
    self->next = nullptr;
    self->prev = nullptr;
    self->data = DATA_NULL;
    self->size = 0;   
}

ListNode* _node_impl_new_node(data) {
    ListNode* new_node = (ListNode*)kmalloc(sizeof(ListNode));
    _node_impl_init(new_node);
    new_node->data = data;
    new_node->size = sizeof(data);
}

// === PUBLIC FUNCTIONS =========================

void node_init(ListNode* self) {
    _node_impl_init(self);
}

ListNode* node_next(ListNode* node) {
    return node->next;
}

ListNode* node_prev(ListNode* node) {
    return node->prev;
}

bool node_has_next(ListNode* node) {
    return (node->next != nullptr);
}

bool node_has_prev(ListNode* node) {
    return (node->prev != nullptr);
}

// *Get the first element of the list
// @param list the list to get the first element from
// @return the first element of the list
ListNode* node_find_origin(ListNode* list) {
    while (node_has_prev(list->prev)) {
        list = list->prev;
    }
    return list;
}

// *Remove the current Node from the list
// @param list the Node to be removed from the list
// @return the previous Node. If there is no previous Node, then the next Node is returned instead. If there is no next Node either, then nullptr is returned.
ListNode* node_remove(ListNode* self) {
    ListNode* ret = nullptr;

    lock(&self->lock);
    
    if (node_has_prev(self)) {                  // if node has prev
        
        lock(&self->prev->lock);
        if (node_has_next(self)) {              // and next
            splice(self->prev, self->next);
        } else 
            terminate(self->prev);
        
        ret = self->prev;
        unlock(&self->prev->lock);
    } else {                                    // node hasn't prev
        if (node_has_next(self)) {              // and next
            lock(&self->next->lock);
            originate(self->next);
            ret = self->next;
            unlock(&self->next->lock);
        }
    }
    
    kfree(self);
    return ret;
}

// *Insert a new Node before the current Node
// @param list the list to insert the new Node into
// @param new_node_data the value of the new Node to be inserted
// @return the pointer to the newly created Node
ListNode* node_insert(list, new_node_data) {
    ListNode* self = (ListNode*)list;
    ListNode* node = _node_impl_new_node(new_node_data);
    lock(&node->lock);
    lock(&self->lock);

    if (node_has_prev(self)) {
        lock(&self->prev->lock);
        self->prev->next = node;
        node->prev = self->prev;
        self->prev = node;
        node->next = self;
        unlock(&self->prev->lock);
    } else {
        self->prev = node;
        node->next = self;
    }

    unlock(&self->lock);
    unlock(&node->lock);
    return node;
}

// *Add a new Node after the current Node
// @param list the list to add the new Node into
// @param new_node_data the value of the new Node to be inserted
// @return the pointer to the newly created Node
ListNode* node_add(list, new_node_data) {
    ListNode* self = (ListNode*)list;
    ListNode* node = _node_impl_new_node(new_node_data);
    lock(&node->lock);
    lock(&self->lock);

    if (node_has_next(self)) {
        lock(&self->next->lock);
        self->next->prev = node;
        node->next = self->next;
        self->next = node;
        node->prev = self;
        unlock(&self->prev->lock);
    } else {
        self->next = node;
        node->prev = self;
    }

    unlock(&self->lock);
    unlock(&node->lock);
    return node;
}

bool list_is_empty(List* list) {
    return (list->size == 0 && list->first == nullptr && list->last == nullptr);
}

// *Destroy all the nodes in the list
// @param list the list to destroy
void list_destroy(List* list) {
    ListNode* p = list->first;
    while (p != nullptr && node_has_next(p)) {
        p = node_remove(p);
    }
}

ListNode* list_add(list, new_node_data) {
    List* self = (List*)list;
    self->last = node_add(self->last, new_node_data);
    self->size++;

    return self->last;
}

ListRemoveResult list_remove_at(List* self, uint32_t index) {
    if (index >= self->size) return OUT_OF_RANGE;
    
    ListNode* node = self->first;
    for (uint32_t i = 0; i <= index; i++) node = node->next;
    node = node_remove(node);
    self->size--;

    if (!node_has_prev(node)) self->first = node;
    else if (!node_has_next(node)) self->last = node;

    return SUCCESS;
}
