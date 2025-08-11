//
// Created by droc101 on 4/21/2024.
//

#ifndef GAME_LIST_H
#define GAME_LIST_H

#include <assert.h>
#include <SDL_mutex.h>

typedef struct List List;
typedef struct LockingList LockingList;

enum _ListType
{
	LIST_POINTER,
	LIST_UINT64,
	LIST_UINT32,
	LIST_INT32,
};

struct _ListData
{
	/// The type of data the list is storing
	enum _ListType type;
	union
	{
		void **pointerData;
		uint64_t *uint64Data;
		uint32_t *uint32Data;
		int32_t *int32Data;
	};
};

struct List
{
	/// The data that the list is storing
	struct _ListData *data;
	/// The number of slots that are actually in use
	size_t length;
};

struct LockingList
{
	/// The data that the list is storing
	struct _ListData *data;
	/// The number of slots that are actually in use
	size_t length;
	/// The mutex used to ensure synchronization across threads
	SDL_mutex *mutex;
};


void _ListInit(List *list, enum _ListType);
void _LockingListInit(LockingList *list, enum _ListType);

void _ListCopy(const List *oldList, List *newList);
void _LockingListCopy(const LockingList *oldList, LockingList *newList);

void _ListAdd(List *list, void *data);
void _LockingListAdd(LockingList *list, void *data);

void _ListSet(const List *list, size_t index, void *data);
void _LockingListSet(const LockingList *list, size_t index, void *data);

void _ListRemoveAt(List *list, size_t index);
void _LockingListRemoveAt(LockingList *list, size_t index);

void _ListInsertAfter(List *list, size_t index, void *data);
void _LockingListInsertAfter(LockingList *list, size_t index, void *data);

size_t _ListFind(const List *list, const void *data);
size_t _LockingListFind(LockingList *list, const void *data);

void _ListLock(const LockingList *list);

void _ListUnlock(const LockingList *list);

void _ListClear(List *list);
void _LockingListClear(LockingList *list);

void _ListZero(const List *list);
void _LockingListZero(const LockingList *list);

void _ListFree(List *list);
void _LockingListFree(LockingList *list);

void _ListFreeOnlyContents(const List *list);
void _LockingListFreeOnlyContents(const LockingList *list);

void _ListAndContentsFree(List *list);
void _LockingListAndContentsFree(LockingList *list);


/**
 * Create a new list of a given size, with zeroed data
 * @param list A pointer to the list object to initialize
 * @param type A value indicating what type of data the list is storing
 */
#define ListInit(list, type) _Generic((list), List: _ListInit, LockingList: _LockingListInit)(&(list), type)

/**
 * Create a copy of a list
 * @param oldList The list to make a copy of
 * @param newList The list to copy into
 */
#define ListCopy(oldList, newList) \
	({ \
		static_assert(sizeof(oldList) == sizeof(newList)); \
		_Generic((oldList), List: _ListCopy, LockingList: _LockingListCopy)(&(oldList), &(newList)); \
	}); \
	(void)0

/**
 * Append an item to the list
 * @param list List to append to
 * @param data Data to append
 */
#define ListAdd(list, data) \
	_Generic((list), List: _ListAdd, LockingList: _LockingListAdd)(&(list), (void *)(uintptr_t)(data))

/**
 * Remove an item from the list by index
 * @param list List to remove from
 * @param index Index to remove
 */
#define ListRemoveAt(list, index) \
	_Generic((list), List: _ListRemoveAt, LockingList: _LockingListRemoveAt)(&(list), (index))

/**
 * Insert an item after a node
 * @param list List to insert into
 * @param index Index to insert after
 * @param data Data to insert
 */
#define ListInsertAfter(list, index, data) \
	_Generic((list), List: _ListInsertAfter, LockingList: _LockingListInsertAfter)(&(list), \
																				   (index), \
																				   (void *)(uintptr_t)(data))

/**
* Get an item of type @code void *@endcode from the list by index
* @param list The list to get from
* @param index The index to get
*/
#define ListGetPointer(list, index) \
	(_Generic((list), \
			 List: (assert((list).data->type == LIST_POINTER), (list).data->pointerData), \
			 LockingList: (assert((list).data->type == LIST_POINTER), (list).data->pointerData))[index])

/**
* Get an item of type @c uint64_t from the list by index
* @param list The list to get from
* @param index The index to get
*/
#define ListGetUint64(list, index) \
	(_Generic((list), \
			 List: (assert((list).data->type == LIST_UINT64), (list).data->uint64Data), \
			 LockingList: (assert((list).data->type == LIST_UINT64), (list).data->uint64Data))[index])

/**
* Get an item of type @c uint32_t from the list by index
* @param list The list to get from
* @param index The index to get
*/
#define ListGetUint32(list, index) \
	(_Generic((list), \
			 List: (assert((list).data->type == LIST_UINT32), (list).data->uint32Data), \
			 LockingList: (assert((list).data->type == LIST_UINT32), (list).data->uint32Data))[index])

/**
* Get an item of type @c int32_t from the list by index
* @param list The list to get from
* @param index The index to get
*/
#define ListGetInt32(list, index) \
	(_Generic((list), \
			 List: (assert((list).data->type == LIST_INT32), (list).data->int32Data), \
			 LockingList: (assert((list).data->type == LIST_INT32), (list).data->int32Data))[index])

/**
 * Set an item in the list by index
 * @param list List to set the item in
 * @param index Index to set
 * @param value Value to set at the index
 */
#define ListSet(list, index, value) \
	((list).data->type == LIST_POINTER \
			 ? (void)((list).data->pointerData[index] = (void *)(uintptr_t)(value)) \
			 : ((list).data->type == LIST_UINT64 \
						? (void)((list).data->uint64Data[index] = (uint64_t)(uintptr_t)(value)) \
						: ((list).data->type == LIST_UINT32 \
								   ? (void)((list).data->uint32Data[index] = (uint32_t)(uintptr_t)(value)) \
								   : (void)(assert((list).data->type == LIST_INT32), \
											(list).data->int32Data[index] = (int32_t)(uintptr_t)(value)))))

/**
 * Find an item in the list
 * @param list List to search
 * @param data Data to search for
 * @return Index of the item in the list, -1 if not found
 */
#define ListFind(list, data) \
	_Generic((list), List: _ListFind, LockingList: _LockingListFind)(&(list), (void *)(uintptr_t)(data))

/**
 * Lock the mutex on a list
 * @param list The list to lock
 */
#define ListLock(list) _Generic((list), LockingList: _ListLock)(&(list))

/**
 * Unlock the mutex on a list
 * @param list The list to unlock
 */
#define ListUnlock(list) _Generic((list), LockingList: _ListUnlock)(&(list))

/**
 * Clear all items from the list
 * @param list List to clear
 * @warning This does not free the data in the list
 */
#define ListClear(list) _Generic((list), List: _ListClear, LockingList: _LockingListClear)(&(list))

/**
 * Sets all items in the list to zero
 * @param list List to zero
 * @warning This does not free the data in the list
 */
#define ListZero(list) _Generic((list), List: _ListZero, LockingList: _LockingListZero)(&(list))

/**
 * Free the list structure
 * @param list List to free
 * @warning This does not free the data in the list, but does free the data pointer
 */
#define ListFree(list) _Generic((list), List: _ListFree, LockingList: _LockingListFree)(&(list))

/**
 * Free the data stored in the list
 * @param list List to free
 */
#define ListFreeOnlyContents(list) \
	_Generic((list), List: _ListFreeOnlyContents, LockingList: _LockingListFreeOnlyContents)(&(list))

/**
 * Free the list structure and the data in the list
 * @param list List to free
 * @warning If the data is a struct, any pointers in the struct will not be freed, just the struct itself
 */
#define ListAndContentsFree(list) \
	_Generic((list), List: _ListAndContentsFree, LockingList: _LockingListAndContentsFree)(&(list))


/**
 * Reallocates memory for an array of arrayLength elements of size bytes each.
 * @param ptr Pointer to the memory block to be reallocated.
 * @param arrayLength Number of elements.
 * @param elementSize Size of each element.
 * @return Pointer to the reallocated memory block.
 */
void *GameReallocArray(void *ptr, size_t arrayLength, size_t elementSize);

#endif //GAME_LIST_H
