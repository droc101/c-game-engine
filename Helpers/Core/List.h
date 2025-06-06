//
// Created by droc101 on 4/21/2024.
//

#ifndef GAME_LIST_H
#define GAME_LIST_H

#include <SDL_mutex.h>
#include <stdbool.h>

typedef struct List List;

struct List
{
	/// The number of slots that are actually in use
	size_t length;
	/// The data that the list is storing
	void **data;
	/// Fix a synchronization segfault
	SDL_mutex *mutex;
};

/**
 * Create a new list of a given size, with zeroed data
 * @param list A pointer to the list object to initialize
 */
void ListCreate(List *list);

/**
 * Append an item to the list
 * @param list List to append to
 * @param data Data to append
 */
void ListAdd(List *list, void *data);

/**
 * Append a group of items to a list
 * @param list The list to append the values to
 * @param count The number of items to append to the list
 * @param ... The items to append to the list
 */
void ListAddBatched(List *list, size_t count, ...);

/**
 * Set an item in the list by index
 * @param list List to set the item in
 * @param index Index to set
 * @param data Data to set at the index
 */
void ListSet(List *list, size_t index, void *data);

/**
 * Remove an item from the list by index
 * @param list List to remove from
 * @param index Index to remove
 */
void ListRemoveAt(List *list, size_t index);

/**
 * Insert an item after a node
 * @param list List to insert into
 * @param index Index to insert after
 * @param data Data to insert
 */
void ListInsertAfter(List *list, size_t index, void *data);

/**
 * Find an item in the list
 * @param list List to search
 * @param data Data to search for
 * @return Index of the item in the list, -1 if not found
 */
size_t ListFind(List list, const void *data);

/**
 * Lock the mutex on a list
 * @param list The list to lock
 */
void ListLock(List list);

/**
 * Unlock the mutex on a list
 * @param list The list to unlock
 */
void ListUnlock(List list);

/**
 * Clear all items from the list
 * @param list List to clear
 * @warning This does not free the data in the list
 */
void ListClear(List *list);

/**
 * Free the list structure
 * @param list List to free
 * @param freeListPointer A boolean indicating if the pointer passed to the first argument should be freed.
 * @warning This does not free the data in the list, but does free the data pointer
 */
void ListFree(List *list, bool freeListPointer);

/**
 * Free the data stored in the list
 * @param list List to free
 */
void ListFreeOnlyContents(List list);

/**
 * Free the list structure and the data in the list
 * @param list List to free
 * @param freeListPointer A boolean indicating if the pointer passed to the first argument should be freed.
 * @warning If the data is a struct, any pointers in the struct will not be freed, just the struct itself
 */
void ListAndContentsFree(List *list, bool freeListPointer);

/**
 * Get an item from the list by index
 * @param list The list to get from
 * @param index The index to get
 */
#define ListGet(list, index) (list).data[(index)]

/**
 * Reallocates memory for an array of arrayLength elements of size bytes each.
 * @param ptr Pointer to the memory block to be reallocated.
 * @param arrayLength Number of elements.
 * @param elementSize Size of each element.
 * @return Pointer to the reallocated memory block.
 */
void *GameReallocArray(void *ptr, size_t arrayLength, size_t elementSize);

#endif //GAME_LIST_H
