#pragma once

#include <cstdint>

template<typename T>
class FastArrayList
{
public:
	FastArrayList(uint32_t sz);
	~FastArrayList();

	T* operator[](uint32_t index); // BEWARE: RETURNS POINTERS TO THE LIST OBJECTS!!

	int32_t push_back(T item);
	int32_t remove(int32_t index);
	int32_t size();
private:
	T * list;
	int32_t list_size;
};

template<typename T>
FastArrayList<T>::FastArrayList(uint32_t sz)
{
	list = new T[sz];
}

template<typename T>
FastArrayList<T>::~FastArrayList()
{
}

template<typename T>
T * FastArrayList<T>::operator[](uint32_t index)
{
	return list + index;
}

template<typename T>
int32_t FastArrayList<T>::push_back(T item)
{
	list[list_size] = item;
	list_size++;
	return list_size;
}

template<typename T>
int32_t FastArrayList<T>::remove(int32_t index)
{
	if (index == list_size - 1) list_size -= 1; // Dont have to copy anything
	else if (index >= list_size || index < 0) return -1; // Not a valid index
	else {
		for (int32_t i = index; i < list_size-1; i++) {
			memcpy(list + i, list + i + 1, sizeof(T));
		}
		list_size -= 1;
	}
	return list_size;
}

template<typename T>
int32_t FastArrayList<T>::size()
{
	return list_size;
}