#pragma once

#include <cstdint>
#include <cstring>

//////////////////////////////////////////////////
// Written by Marco Satti.                      //
// Licensed under GPLv3. See LICENSE file.      //
// Designed to be fast: minimal error checking! //
//////////////////////////////////////////////////

template<typename T>
class FastArrayList
{
public:
	// Array supports up to SIZE_MAX - 1. SIZE_MAX (which is equal to -1 when signed) is reserved for 'item not found' error.
	// Array may fail allocation if sz is too large, but this is platform dependent.
	FastArrayList(size_t sz); // Constructs and allocates an array of specified size (static). Does not support dynamically allocated storage.
	~FastArrayList(); // Deletes array and deconstructs.

	size_t push_back(T item); // Push an item onto the back of the array.
	T pop_back(); // Returns the item currently on the back of the array and removes it.
	size_t size(); // Returns the number of items currently in the array.
	size_t remove(size_t index); // Removes an item from the array and shifts the other items to the front. Returns new array size. Does not reformat memory, but may get overwritten by items to the right.
	size_t insert(size_t index, T item); // Inserts an item at specified index, copying other items to the right.
	size_t find(T item); // Iterates through the array, using memcmp to test if two items are the same. Returns index if match is found and SIZE_MAX if not found. WARNING: USE WITH CARE. ADVISABLE TO NOT CACHE RESULTS. IF ARRAY IS ALTERED, THERE IS NO GUARANTEE THE POINTER WILL BE VALID.
	T get(size_t index); // Returns a copy of the item requested.
	T* get_ptr(size_t index); // Returns a pointer to the item requested. WARNING: USE WITH CARE. ADVISABLE TO NOT CACHE RESULTS. IF ARRAY IS ALTERED, THERE IS NO GUARANTEE THE POINTER WILL BE VALID.

private:
	T * list;
	size_t list_ptr;
};

template<typename T>
FastArrayList<T>::FastArrayList(size_t sz)
{
	if (sz >= SIZE_MAX - 1) throw NULL;
	list = new T[sz];
	memset(list, 0, sz * sizeof(T));
	list_ptr = 0;
}

template<typename T>
FastArrayList<T>::~FastArrayList()
{
	delete list;
}

template<typename T>
inline size_t FastArrayList<T>::push_back(T item)
{
	// Slower ?
	//list[list_size] = item;

	// Faster ? might be due to something else ie: memory layout
	memcpy(&list[list_ptr], &item, sizeof(T));

	list_ptr++;
	return list_ptr - 1;
}

template<typename T>
inline T FastArrayList<T>::pop_back()
{
	list_ptr -= 1; // Dont have to copy anything as its the last item.
	return list[list_ptr];
}

template<typename T>
inline size_t FastArrayList<T>::remove(size_t index)
{
	if (index == list_ptr - 1) list_ptr -= 1; // Dont have to copy anything
	else if (index >= list_ptr || index == (size_t) -1) return (size_t) -1; // Not a valid index
	else {
		for (size_t i = index; i < list_ptr - 1; i++) {
			memcpy(&list[i], &list[i+1], sizeof(T));
		}
		list_ptr -= 1;
	}
	return list_ptr;
}

template<typename T>
inline size_t FastArrayList<T>::insert(size_t index, T item)
{
	if (index > list_ptr) return (size_t) -1; // Not valid.
	// Need to iterate from last item to (index) so no data is lost.
	for (size_t i = list_ptr; i > index; i--) {
		memcpy(&list[i], &list[i-1], sizeof(T));
	}
	memcpy(&list[index], &item, sizeof(T)); // Copy the item parsed into index.
	list_ptr++;
	return list_ptr;
}

template<typename T>
inline size_t FastArrayList<T>::size()
{
	return list_ptr;
}

template<typename T>
inline size_t FastArrayList<T>::find(T item)
{
	for (size_t i = 0; i < list_ptr; i++) {
		if (memcmp(&item, &list[i], sizeof(T)) == 0) {
			return i;
		}
	}
	return (size_t) -1;
}

template<typename T>
inline T FastArrayList<T>::get(size_t index)
{
	return list[index];
}

template<typename T>
inline T * FastArrayList<T>::get_ptr(size_t index)
{
	return &list[index];
}
