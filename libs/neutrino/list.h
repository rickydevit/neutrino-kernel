#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <_null.h>
#include <neutrino/macros.h>
#include <neutrino/lock.h>

typedef struct __list_node {
    void* data;
    uint64_t size;

    struct __list_node* next;
    struct __list_node* prev;

    Lock lock;
} packed ListNode;

typedef struct __list {
    ListNode* first;
    ListNode* last;
    uint32_t size;
} packed List;

#define NewList     (List){nullptr, nullptr, 0}
#define DATA_NULL 0 

typedef enum __list_remove_result {
    SUCCESS,
    OUT_OF_RANGE,
} ListRemoveResult;

#define TypedListNode(T)                                \
        union {                                         \
            struct {                                    \
                T data;                                 \
                uint64_t size;                          \
            };                                          \
            ListNode _node;                             \
        }                                               \

#define splice(a, b)            a->next = b; b->prev = a
#define originate(a)            a->prev = nullptr
#define terminate(a)            a->next = nullptr

void node_init(ListNode* self);

ListNode* node_next(ListNode* node);
ListNode* node_prev(ListNode* node);
bool node_has_next(ListNode* node);
bool node_has_prev(ListNode* node);
ListNode* node_remove(node);
ListNode* node_insert(node, new_node_data);
ListNode* node_add(node, new_node_data);

bool list_is_empty(List* list);
void list_destroy(List* list);
ListNode* list_add(list, new_node_data);
ListRemoveResult list_remove_at(List* self, uint32_t index);
