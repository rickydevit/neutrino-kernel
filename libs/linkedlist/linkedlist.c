#include <stdint.h>
#include <stdbool.h>
#include <_null.h>
#include <liballoc.h>
#include "../linkedlist.h"

List* list_create_node(void* data) {
        List* new_node = (List*)lmalloc(sizeof(List));
        new_node->next = nullptr;
        new_node->prev = nullptr;
        new_node->data = data;
        return new_node;
}

bool list_is_empty(List* list) {
        return list == nullptr;
}

bool list_is_not_null(List* list) {
        return list != nullptr;
}

bool list_has_next(List* node) {
        return node->next != nullptr;
}

List* list_prepend(List* list, void* data) {
        List* new_node = list_create_node(data);
        if (list_is_empty(list)) {
                return new_node;
        } else {
                new_node->next = list;
                list->prev = new_node;
        }
        return new_node;
}

List* list_append(List* list, void* data) {
        List* new_node = list_create_node(data);
        if (list_is_empty(list)) {
                return new_node;
        } else {
                List* temp = list;
                while (list_has_next(temp)) {
                        temp = temp->next;
                }
                temp->next = new_node;
                new_node->prev = temp;
        }
        return list;
}

List* list_get_head(List* list) {
        if (list == nullptr || list->prev == nullptr)
                return list;
        return list_get_head(list->prev);
}

List* list_get_tail(List* list) {
        if (list_is_empty(list)) {
                return nullptr;
        } else {
                List* temp = list;
                while (list_has_next(temp)) {
                        temp = temp->next;
                }
                return temp;
        }
}

List* list_delete_head(List* list) {
        if (list_is_empty(list)) {
                return nullptr;
        } else {
            if (list->next == nullptr) {
                lfree(list);
                return nullptr;
            } else {
                list = list->next;
                lfree(list->prev);
                list->prev = nullptr;
            }                
        }
        return list;
}

List* list_delete_tail(List* list) {
        if (list_is_empty(list)) {
                return nullptr;
        } else {
                List* temp = list;
                while (list_has_next(temp)) {
                        temp = temp->next;
                }
                temp->prev->next = nullptr;
                lfree(temp->prev->next);
                return list;
        }
}

int list_get_size(List* list) {
        int size = 0;
        if (list_is_empty(list)) {
                return size;
        } else {
                List* temp =  list;
                while (list_is_not_null(temp)) {
                        size++;
                        temp = temp->next;
                }
        }
        return size;
}

void* list_get_value(List* node) {
        return node->data;
}

bool list_is_last(List* node, int iterator) {
        return iterator == list_get_size(node);
}

bool list_is_first(int iterator) {
        return iterator == 0;
}

List* list_insert_at(List* list, int index, void* data) {
        if (index <= list_get_size(list) && index >= 0) {
                int iterator = 0;
                List* temp = list;
                if (list_is_first(index)) {
                        return list_prepend(list, data);
                } else if (list_is_last(list, index)) {
                        return list_append(list, data);
                }
                while (list_is_not_null(temp)) {
                        List* new_node = list_create_node(data);
                        if (iterator == index) {
                                temp = temp->prev;
                                temp->next->prev = new_node;
                                new_node->next = temp->next;
                                new_node->prev = temp;
                                temp->next = new_node;
                                return list;
                        }
                        iterator++;
                        temp = temp->next;
                }
        }
        return list;
}

List* list_delete_at(List* list, int index) {
        if (index <= list_get_size(list) && index >= 0) {
                int iterator = 0;
                List* temp = list;
                if (list_is_first(index)) {
                        return list_delete_head(list);
                } else if(list_is_last(list, index)) {
                        return list_delete_tail(list);
                }
                while (list_is_not_null(temp)) {
                        if(iterator == index) {
                                if (list_has_next(temp)) temp->next->prev = temp->prev;
                                if (list_has_prev(temp)) temp->prev->next = temp->next;
                                lfree(temp);
                                return list;
                        }
                        iterator++;
                        temp = temp->next;
                }
        }
        return list;
}

List* list_find_node_with(List* list, void* data) {
        if (list_is_empty(list)) {
                return nullptr;
        } else {
                List* temp = list;
                while (list_is_not_null(temp)) {
                        if (temp->data == data) {
                                return temp;
                        }
                        temp = temp->next;
                }
                return nullptr;
        }
}

List* list_find_at(List* list, int index) {
        if (index <= list_get_size(list) && index >= 0) {
                if (list_is_empty(list)) {
                        return nullptr;
                } else {
                        int iterator = 0;
                        List* temp = list;
                        while (list_is_not_null(temp)) {
                                if (iterator == index) {
                                        return temp;
                                } else {
                                        iterator++;
                                        temp = temp->next;
                                }
                        }
                }
        }
        return nullptr;
}

bool list_has_prev(List* node) {
        return node->prev != nullptr;
}

List* list_concatenate(List* list1, List* list2) {
        List* last_node = list_get_tail(list1);
        last_node->next = list2;
        list2->prev = last_node;
        return list1;
}