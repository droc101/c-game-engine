//
// Created by droc101 on 4/21/2024.
//

#ifndef GAME_LIST_H
#define GAME_LIST_H

#include <assert.h>
#include <SDL_mutex.h>

typedef struct List List;
typedef struct SortedList SortedList;
typedef struct LockingList LockingList;
typedef struct LockingSortedList LockingSortedList;

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
		size_t *uint64Data;
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

struct SortedList
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

struct LockingSortedList
{
	/// The data that the list is storing
	struct _ListData *data;
	/// The number of slots that are actually in use
	size_t length;
	/// The mutex used to ensure synchronization across threads
	SDL_mutex *mutex;
};


void _ListInit(List *list, enum _ListType);
void _SortedListInit(SortedList *list, enum _ListType);
void _LockingListInit(LockingList *list, enum _ListType);
void _LockingSortedListInit(LockingSortedList *list, enum _ListType);

void _ListAdd(List *list, void *data);
void _SortedListAdd(const SortedList *list, void *data);
void _LockingListAdd(LockingList *list, void *data);
void _LockingSortedListAdd(LockingSortedList *list, void *data);

void _ListSet(const List *list, size_t index, void *data);
void _LockingListSet(const LockingList *list, size_t index, void *data);

void _ListRemoveAt(List *list, size_t index);
void _SortedListRemoveAt(SortedList *list, size_t index);
void _LockingListRemoveAt(LockingList *list, size_t index);
void _LockingSortedListRemoveAt(LockingSortedList *list, size_t index);

void _ListInsertAfter(List *list, size_t index, void *data);
void _LockingListInsertAfter(LockingList *list, size_t index, void *data);

size_t _ListFind(const List *list, const void *data);
size_t _SortedListFind(const SortedList *list, const void *data);
size_t _LockingListFind(LockingList *list, const void *data);
size_t _LockingSortedListFind(LockingSortedList *list, const void *data);

void _ListLock(const LockingList *list);
void _SortedListLock(LockingSortedList *list);

void _ListUnlock(const LockingList *list);
void _SortedListUnlock(LockingSortedList *list);

void _ListClear(List *list);
void _SortedListClear(SortedList *list);
void _LockingListClear(LockingList *list);
void _LockingSortedListClear(LockingSortedList *list);

void _ListFree(List *list);
void _SortedListFree(SortedList *list);
void _LockingListFree(LockingList *list);
void _LockingSortedListFree(LockingSortedList *list);

void _ListFreeOnlyContents(const List *list);
void _LockingListFreeOnlyContents(const LockingList *list);

void _ListAndContentsFree(List *list);
void _LockingListAndContentsFree(LockingList *list);


#define _ListDataHelper(list) \
	((list).data->type == LIST_POINTER \
			 ? (list).data->pointerData \
			 : ((list).data->type == LIST_UINT64 \
						? (list).data->uint64Data \
						: ((list).data->type == LIST_UINT32 \
								   ? (list).data->uint32Data \
								   : (assert((list).data->type == LIST_INT32), (list).data->int32Data))))

/**
 * Create a new list of a given size, with zeroed data
 * @param list A pointer to the list object to initialize
 */
#define ListInit(list) \
	_Generic((list), \
			List: _ListInit, \
			SortedList: _SortedListInit, \
			LockingList: _LockingListInit, \
			LockingSortedList: _LockingSortedListInit)(&(list), LIST_POINTER)

/**
 * Append an item to the list
 * @param list List to append to
 * @param data Data to append
 */
#define ListAdd(list, data) \
	_Generic((list), \
			List: _ListAdd, \
			SortedList: _SortedListAdd, \
			LockingList: _LockingListAdd, \
			LockingSortedList: _LockingSortedListAdd)(&(list), (void *)(size_t)(data))

/**
 * Remove an item from the list by index
 * @param list List to remove from
 * @param index Index to remove
 */
#define ListRemoveAt(list, index) \
	_Generic((list), \
			List: _ListRemoveAt, \
			SortedList: _SortedListRemoveAt, \
			LockingList: _LockingListRemoveAt, \
			LockingSortedList: _LockingSortedListRemoveAt)(&(list), (index))

/**
 * Insert an item after a node
 * @param list List to insert into
 * @param index Index to insert after
 * @param data Data to insert
 */
#define ListInsertAfter(list, index, data) \
	_Generic((list), List: _ListInsertAfter, LockingList: _LockingListInsertAfter, )(&(list), \
																					 (index), \
																					 (void *)(size_t)(data))

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
			 SortedList: (assert((list).data->type == LIST_UINT64), (list).data->uint64Data), \
			 LockingList: (assert((list).data->type == LIST_UINT64), (list).data->uint64Data), \
			 LockingSortedList: (assert((list).data->type == LIST_UINT64), (list).data->uint64Data))[index])

/**
* Get an item of type @c uint32_t from the list by index
* @param list The list to get from
* @param index The index to get
*/
#define ListGetUint32(list, index) \
	(_Generic((list), \
			 List: (assert((list).data->type == LIST_UINT32), (list).data->uint32Data), \
			 SortedList: (assert((list).data->type == LIST_UINT32), (list).data->uint32Data), \
			 LockingList: (assert((list).data->type == LIST_UINT32), (list).data->uint32Data), \
			 LockingSortedList: (assert((list).data->type == LIST_UINT32), (list).data->uint32Data))[index])

/**
* Get an item of type @c int32_t from the list by index
* @param list The list to get from
* @param index The index to get
*/
#define ListGetInt32(list, index) \
	(_Generic((list), \
			 List: (assert((list).data->type == LIST_INT32), (list).data->int32Data), \
			 SortedList: (assert((list).data->type == LIST_INT32), (list).data->int32Data), \
			 LockingList: (assert((list).data->type == LIST_INT32), (list).data->int32Data), \
			 LockingSortedList: (assert((list).data->type == LIST_INT32), (list).data->int32Data))[index])

/**
 * Set an item in the list by index
 * @param list List to set the item in
 * @param index Index to set
 * @param data Data to set at the index
 */
#define ListSet(list, index, data) \
	_Generic((list), List: _ListSet, LockingList: _LockingListSet)(&(list), (index), (data))

/**
 * Find an item in the list
 * @param list List to search
 * @param data Data to search for
 * @return Index of the item in the list, -1 if not found
 */
#define ListFind(list, data) \
	_Generic((list), \
			List: _ListFind, \
			SortedList: _SortedListFind, \
			LockingList: _LockingListFind, \
			LockingSortedList: _LockingSortedListFind)(&(list), (data))

/**
 * Lock the mutex on a list
 * @param list The list to lock
 */
#define ListLock(list) _Generic((list), LockingList: _ListLock, LockingSortedList: _SortedListLock)(&(list))

/**
 * Unlock the mutex on a list
 * @param list The list to unlock
 */
#define ListUnlock(list) _Generic((list), LockingList: _ListUnlock, LockingSortedList: _SortedListUnlock)(&(list))

/**
 * Clear all items from the list
 * @param list List to clear
 * @warning This does not free the data in the list
 */
#define ListClear(list) \
	_Generic((list), \
			List: _ListClear, \
			SortedList: _SortedListClear, \
			LockingList: _LockingListClear, \
			LockingSortedList: _LockingSortedListClear)(&(list))

/**
 * Free the list structure
 * @param list List to free
 * @warning This does not free the data in the list, but does free the data pointer
 */
#define ListFree(list) \
	_Generic((list), \
			List: _ListFree, \
			SortedList: _SortedListFree, \
			LockingList: _LockingListFree, \
			LockingSortedList: _LockingSortedListFree)(&(list))

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
