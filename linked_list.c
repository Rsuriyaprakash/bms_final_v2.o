/** @file linked_list.c @brief Generic linked list implementation. */
#include "linked_list.h"
#include "logging.h"
#include <stdlib.h>
#include <string.h>

static BmsLinkedListNode_t *CreateNode(const BmsLinkedList_t *list, const void *data)
{
    BmsLinkedListNode_t *node;
    if ((list == NULL) || (data == NULL) || (list->elementSize == 0U)) { return NULL; }
    node = (BmsLinkedListNode_t *)calloc(1U, sizeof(*node));
    if (node == NULL) { return NULL; }
    node->data = calloc(1U, list->elementSize);
    if (node->data == NULL) { free(node); return NULL; }
    (void)memcpy(node->data, data, list->elementSize);
    return node;
}

BmsStatus_t LinkedListInitialize(BmsLinkedList_t *list, size_t elementSize)
{
    if ((list == NULL) || (elementSize == 0U)) { return BMS_STATUS_INVALID_ARGUMENT; }
    (void)memset(list, 0, sizeof(*list)); list->elementSize = elementSize;
    return BMS_STATUS_OK;
}
BmsStatus_t LinkedListInsertFront(BmsLinkedList_t *list, const void *data)
{
    BmsLinkedListNode_t *node = CreateNode(list, data);
    if (node == NULL) { return BMS_STATUS_MEMORY_ERROR; }
    node->next = list->head; list->head = node;
    if (list->tail == NULL) { list->tail = node; }
    ++list->count; return BMS_STATUS_OK;
}
BmsStatus_t LinkedListInsertBack(BmsLinkedList_t *list, const void *data)
{
    BmsLinkedListNode_t *node = CreateNode(list, data);
    if (node == NULL) { return BMS_STATUS_MEMORY_ERROR; }
    if (list->tail == NULL) { list->head = node; list->tail = node; }
    else { list->tail->next = node; list->tail = node; }
    ++list->count; return BMS_STATUS_OK;
}
BmsStatus_t LinkedListFind(const BmsLinkedList_t *list, BmsListPredicate_t predicate,
                           const void *key, void **data)
{
    BmsLinkedListNode_t *node;
    if ((list == NULL) || (predicate == NULL) || (data == NULL))
    { return BMS_STATUS_INVALID_ARGUMENT; }
    node = list->head;
    while (node != NULL)
    {
        if (predicate(node->data, key)) { *data = node->data; return BMS_STATUS_OK; }
        node = node->next;
    }
    *data = NULL; return BMS_STATUS_NOT_FOUND;
}
BmsStatus_t LinkedListUpdate(const BmsLinkedList_t *list, BmsListPredicate_t predicate,
                             const void *key, const void *replacement)
{
    void *found = NULL; BmsStatus_t status;
    if ((list == NULL) || (replacement == NULL)) { return BMS_STATUS_INVALID_ARGUMENT; }
    status = LinkedListFind(list, predicate, key, &found);
    if (status == BMS_STATUS_OK) { (void)memcpy(found, replacement, list->elementSize); }
    return status;
}
BmsStatus_t LinkedListDelete(BmsLinkedList_t *list, BmsListPredicate_t predicate,
                             const void *key)
{
    BmsLinkedListNode_t *node, *previous = NULL;
    if ((list == NULL) || (predicate == NULL)) { return BMS_STATUS_INVALID_ARGUMENT; }
    node = list->head;
    while (node != NULL)
    {
        if (predicate(node->data, key))
        {
            if (previous == NULL) { list->head = node->next; }
            else { previous->next = node->next; }
            if (list->tail == node) { list->tail = previous; }
            free(node->data); free(node); --list->count; return BMS_STATUS_OK;
        }
        previous = node; node = node->next;
    }
    return BMS_STATUS_NOT_FOUND;
}
BmsStatus_t LinkedListTraverse(const BmsLinkedList_t *list, BmsListVisitor_t visitor,
                               void *context)
{
    const BmsLinkedListNode_t *node;
    if ((list == NULL) || (visitor == NULL)) { return BMS_STATUS_INVALID_ARGUMENT; }
    node = list->head;
    while (node != NULL)
    {
        BmsStatus_t status = visitor(node->data, context);
        if (status != BMS_STATUS_OK) { return status; }
        node = node->next;
    }
    return BMS_STATUS_OK;
}
BmsStatus_t LinkedListSort(BmsLinkedList_t *list, BmsListCompare_t compare)
{
    bool swapped;

    if ((list == NULL) || (compare == NULL))
    {
        return BMS_STATUS_INVALID_ARGUMENT;
    }
    if (list->count < 2U)
    {
        return BMS_STATUS_OK;
    }

    do
    {
        BmsLinkedListNode_t *node = list->head;
        swapped = false;
        while (node != NULL)
        {
            BmsLinkedListNode_t *next = node->next;
            if (next == NULL)
            {
                break;
            }
            if (compare(node->data, next->data) > 0)
            {
                void *temporary = node->data;
                node->data = next->data;
                next->data = temporary;
                swapped = true;
            }
            node = next;
        }
    } while (swapped);

    return BMS_STATUS_OK;
}
uint32_t LinkedListGetCount(const BmsLinkedList_t *list)
{ return (list == NULL) ? 0U : list->count; }
bool LinkedListIsEmpty(const BmsLinkedList_t *list)
{ return (list == NULL) || (list->count == 0U); }
void LinkedListClear(BmsLinkedList_t *list)
{
    BmsLinkedListNode_t *node, *next;
    if (list == NULL) { return; }
    node = list->head;
    while (node != NULL) { next = node->next; free(node->data); free(node); node = next; }
    list->head = NULL; list->tail = NULL; list->count = 0U;
}
