#ifndef RADIO_LIST_H
#define RADIO_LIST_H

#include <stdbool.h>

#define RADIO_TEXT_SIZE 96

typedef struct Transmission {
    char title[RADIO_TEXT_SIZE];
    char code[RADIO_TEXT_SIZE];
    int deadline;
    int cost;
    bool urgent;
} Transmission;

typedef struct RadioNode {
    Transmission data;
    struct RadioNode *prev;
    struct RadioNode *next;
} RadioNode;

typedef struct {
    RadioNode *head;
    RadioNode *tail;
    int size;
} RadioList;

void radio_list_init(RadioList *list);
RadioNode *radio_list_push_back(RadioList *list, Transmission data);
void radio_list_remove(RadioList *list, RadioNode *node);
void radio_list_move_up(RadioList *list, RadioNode *node);
void radio_list_insertion_sort(RadioList *list);
void radio_list_clear(RadioList *list);

#endif
