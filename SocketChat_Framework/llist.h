//llist.h
#ifndef LLIST_H
#define LLIST_H

#include <stdlib.h>
#include <string.h>
#include "tools.h"

//Structure necessary for TLinkedList to work. Contains pointer to the next node and pointer to data it contains.
typedef struct sLinkedListNode
{
	void* item;
	struct sLinkedListNode* nextNode;
} LinkedListNode;

//Structure which defines one-way linked list. All functions which work with linked list have prefix "llist_".
typedef struct sLinkedList
{
	LinkedListNode* firstNode;
	Comparision* comparer;
	bool byValue;
	size_t itemSize;
	size_t count;
} LinkedList;

//Allocates memory for linked list. A pointer to list is returned, or NULL, if failed.
//Parameter #1 - pointer to function which compares items in list to determine their order;
//Parameter #2 - boolean which defines if list copies values of items, or pointers to original items added;
//Parameter #3 - size of items stored in list (necessary only if byValue == true).
LinkedList* llist_create(Comparision* comparer, bool byValue, const size_t itemSize);

//Add new item to list.
//Parameter #1 - pointer to linked list;
//Parameter #2 - pointer to item to add.
void llist_add(LinkedList* list, const void* item);

//Add new item to list (by pointer). 
//Parameter #1 - pointer to linked list;
//Parameter #2 - pointer to item to add.
void llist_addItem(LinkedList* list, void* item);

//Add new item to list (by value).
//Parameter #1 - pointer to linked list;
//Parameter #2 - pointer to item to add.
void llist_addValue(LinkedList* list, const void* valuePtr);

//Remove existing item from list. If no such item is found, function does nothing. 
//Parameter #1 - pointer to linked list;
//Parameter #2 - pointer to item to remove.
void llist_remove(LinkedList* list, const void* item);

//Remove existing item from list (search by pointer). If no such item is found, function does nothing. 
//Parameter #1 - pointer to linked list;
//Parameter #2 - pointer to item to remove.
void llist_removeItem(LinkedList* list, const void* item);

//Remove existing item from list (search by value). If no such item is found, function does nothing. 
//Parameter #1 - pointer to linked list;
//Parameter #2 - pointer to item to remove.
void llist_removeValue(LinkedList* list, const void* valuePtr);

//Returns true if pointer to item is in the list.
//Parameter #1 - pointer to linked list;
//Parameter #2 - pointer to item to check.
bool llist_containsItem(const LinkedList* list, const void* item);

//Returns true if value of item is in the list.
//Parameter #1 - pointer to linked list;
//Parameter #2 - pointer to item to check.
bool llist_containsValue(const LinkedList* list, const void* valuePtr);

//Changes order in which items are stored in linked list.
//Parameter #1 - pointer to linked list;
//Parameter #2 - pointer to new comparision function.
void llist_rearrange(LinkedList* list, Comparision* comparer);

//Clears list and frees memory allocated.
//Parameter #1 - pointer to linked list.
void llist_dispose(LinkedList* list);

//Removes all items from the list. If items are stored by value, they are freed automaticially.
//Parameter #1 - pointer to linked list.
void llist_clear(LinkedList* list);

//Use this line to iterate through all items in a list. Use:
//llist_foreach(list, prefix) { void* item = prefix_node->item; ... }
//Warning: You shouldn't delete items from list during iteration.
#define llist_foreach(list, prefix) for (LinkedListNode* prefix##_node = list->firstNode; prefix##_node != NULL; prefix##_node = prefix##_node->nextNode)

#endif