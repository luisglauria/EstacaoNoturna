#include "radio_list.h"
#include <stdlib.h>

void radio_list_init(RadioList *list) {
    list->head = NULL;
    list->tail = NULL;
    list->size = 0;
}

RadioNode *radio_list_push_back(RadioList *list, Transmission data) {
    RadioNode *node = malloc(sizeof(*node));
    if (!node) return NULL;
    node->data = data;
    node->next = NULL;
    node->prev = list->tail;
    if (list->tail) list->tail->next = node;
    else list->head = node;
    list->tail = node;
    list->size++;
    return node;
}

void radio_list_remove(RadioList *list, RadioNode *node) {
    if (!node) return;
    if (node->prev) node->prev->next = node->next;
    else list->head = node->next;
    if (node->next) node->next->prev = node->prev;
    else list->tail = node->prev;
    free(node);
    list->size--;
}

void radio_list_move_up(RadioList *list, RadioNode *node) {
    if (!node || !node->prev) return;
    RadioNode *before = node->prev;
    RadioNode *after = node->next;
    node->prev = before->prev;
    node->next = before;
    before->prev = node;
    before->next = after;
    if (after) after->prev = before;
    else list->tail = before;
    if (node->prev) node->prev->next = node;
    else list->head = node;
}

void radio_list_insertion_sort(RadioList *list) {
    RadioNode *current = list->head ? list->head->next : NULL;
    while (current) {
        RadioNode *next = current->next;
        RadioNode *scan = current->prev;
        while (scan && scan->data.deadline > current->data.deadline) {
            Transmission temp = scan->data;
            scan->data = current->data;
            current->data = temp;
            current = scan;
            scan = current->prev;
        }
        current = next;
    }
}

void radio_list_clear(RadioList *list) {
    RadioNode *node = list->head;
    while (node) {
        RadioNode *next = node->next;
        free(node);
        node = next;
    }
    radio_list_init(list);
}
