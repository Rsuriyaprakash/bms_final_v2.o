/**
 * @file linked_list.h
 * @author Suriya Prakash R
 * @version 2.0.0
 * @brief Generic singly linked-list abstraction.
 *
 * Dynamic memory is used only for list nodes and copied record payloads.
 */

#ifndef BMS_LINKED_LIST_H
#define BMS_LINKED_LIST_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "common_error_codes.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct BmsLinkedListNode
{
    void *data;
    struct BmsLinkedListNode *next;
} BmsLinkedListNode_t;

typedef struct
{
    BmsLinkedListNode_t *head;
    BmsLinkedListNode_t *tail;
    size_t elementSize;
    uint32_t count;
} BmsLinkedList_t;

typedef int32_t (*BmsListCompare_t)(const void *left, const void *right);
typedef bool (*BmsListPredicate_t)(const void *data, const void *key);
typedef BmsStatus_t (*BmsListVisitor_t)(void *data, void *context);

BmsStatus_t LinkedListInitialize(BmsLinkedList_t *list, size_t elementSize);
BmsStatus_t LinkedListInsertFront(BmsLinkedList_t *list, const void *data);
BmsStatus_t LinkedListInsertBack(BmsLinkedList_t *list, const void *data);
BmsStatus_t LinkedListFind(const BmsLinkedList_t *list,
                           BmsListPredicate_t predicate,
                           const void *key,
                           void **data);
BmsStatus_t LinkedListUpdate(const BmsLinkedList_t *list,
                             BmsListPredicate_t predicate,
                             const void *key,
                             const void *replacement);
BmsStatus_t LinkedListDelete(BmsLinkedList_t *list,
                             BmsListPredicate_t predicate,
                             const void *key);
BmsStatus_t LinkedListTraverse(const BmsLinkedList_t *list,
                               BmsListVisitor_t visitor,
                               void *context);
BmsStatus_t LinkedListSort(BmsLinkedList_t *list, BmsListCompare_t compare);
uint32_t LinkedListGetCount(const BmsLinkedList_t *list);
bool LinkedListIsEmpty(const BmsLinkedList_t *list);
void LinkedListClear(BmsLinkedList_t *list);

#ifdef __cplusplus
}
#endif

#endif /* BMS_LINKED_LIST_H */
