#pragma once

#include <cstdint>

template<typename T>
class FastArrayList
{
public:
	FastArrayList(uint32_t sz);
	~FastArrayList();

	int32_t push_back(T item);
	int32_t remove(int32_t index);
	int32_t size();
	int32_t find(T item);
	T get(int32_t index); // dereferences an item and returns it
	T* get_ptr(uint32_t index); // BEWARE: RETURNS POINTERS TO THE LIST OBJECTS!!

private:
	T * list;
	int32_t list_size;
};

template<typename T>
FastArrayList<T>::FastArrayList(uint32_t sz)
{
	list = new T[sz];
	list_size = 0;
}

template<typename T>
FastArrayList<T>::~FastArrayList()
{
	delete list;
}

template<typename T>
inline int32_t FastArrayList<T>::push_back(T item)
{
	// Slower ?
	//list[list_size] = item;
	// Faster ? might be due to something else ie: memory layout
	memcpy(list + list_size, &item, sizeof(T));
	list_size++;
	return list_size - 1;
}

template<typename T>
inline int32_t FastArrayList<T>::remove(int32_t index)
{
	if (index == list_size - 1) list_size -= 1; // Dont have to copy anything
	else if (index >= list_size || index < 0) return -1; // Not a valid index
	else {
		for (int32_t i = index; i < list_size - 1; i++) {
			memcpy(list + i, list + i + 1, sizeof(T));
		}
		list_size -= 1;
	}
	return list_size;
}

template<typename T>
inline int32_t FastArrayList<T>::size()
{
	return list_size;
}

template<typename T>
inline int32_t FastArrayList<T>::find(T item)
{
	for (int32_t i = 0; i < list_size; i++) {
		if (memcmp(&item, &list[i], sizeof(T)) == 0) {
			return i;
		}
	}
	return -1;
}

template<typename T>
inline T FastArrayList<T>::get(int32_t index)
{
	return list[index];
}

template<typename T>
inline T * FastArrayList<T>::get_ptr(uint32_t index)
{
	return list + index;
}
