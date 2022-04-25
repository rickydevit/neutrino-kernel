#pragma once
#include <stdbool.h>

struct _List {
    void *data;
    struct _List *next;
    struct _List *prev;
};

typedef struct _List List;

List *list_create_node(void *data);
bool list_is_empty(List *list);
bool list_is_not_null(List *node);
bool list_has_next(List *node);
List *list_prepend(List *list,
                   void *data);
List *list_append(List *list,
                  void *data);
List *list_get_head(List *list);
List *list_get_tail(List *list);
List *list_delete_head(List *list);
List *list_delete_tail(List *list);
int list_get_size(List *list);
void *list_get_value(List *node);
bool list_is_last(List *node,
                  int iterator);
bool list_is_first(int iterator);
List *list_insert_at(List *list,
                     int index,
                     void *data);
List *list_delete_at(List *list,
                     int index);
List *list_find_node_with(List *list,
                          void *data);
List *list_find_at(List *list,
                   int index);
bool list_has_prev(List *node);
List *list_concatenate(List *list1,
                       List *list2);
