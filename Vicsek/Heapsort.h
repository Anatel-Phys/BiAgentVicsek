#pragma once
#include <vector>
#include <codecvt>

#define right(i) (2*i + 2)
#define left(i) (2*i + 1)
#define parent(i) (i/2)

struct CoordWithKey
{
	size_t x;
	size_t y;
	int key;
};

inline int up_round(float x)
{
	return (int)x + (int)(x > (int)x);
}

inline void max_heapify(std::vector<CoordWithKey>* array, size_t i, size_t heapSize)
{
	size_t l = left(i);
	size_t r = right(i);
	size_t largest;

	if (l < heapSize && array->at(i).key < array->at(l).key)
		largest = l;
	else
		largest = i;
	if (r < heapSize && array->at(largest).key < array->at(r).key)
		largest = r;

	if (largest != i)
	{
		CoordWithKey buffer = array->at(i);
		array->at(i) = array->at(largest);
		array->at(largest) = buffer;
		max_heapify(array, largest, heapSize);
	}
}

inline void build_max_heap(std::vector<CoordWithKey>* array, size_t arraySize)
{
	for (int i = up_round((float)arraySize / 2) - 1; i >= 0; i--)
		max_heapify(array, i, arraySize);
}

inline void heap_sort(std::vector<CoordWithKey>* array, size_t arraySize)
{
	build_max_heap(array, arraySize);
	size_t heapSize = arraySize;
	CoordWithKey valBuffer;

	while (heapSize > 1)
	{
		valBuffer = array->at(heapSize - 1);
		array->at(heapSize - 1) = array->at(0);
		array->at(0) = valBuffer;
		heapSize--;
		max_heapify(array, 0, heapSize);
	}

}

template<class T>
struct ptrWithKey
{
	T* obj;
	size_t key;

	ptrWithKey<T>& operator=(const ptrWithKey<T>& other)
	{
		this->obj = other.obj;
		this->key = other.key;

		return *this;
	}

	ptrWithKey()
	{
		this->obj = nullptr;
		this->key = 0;
	}

	ptrWithKey(T* _obj, size_t _key)
	{
		this->obj = _obj;
		this->key = _key;
	}
};

template<class T>
inline void max_heapify_g(std::vector<ptrWithKey<T>>* array, size_t i, size_t heapSize)
{
	size_t l = left(i);
	size_t r = right(i);
	size_t largest;

	if (l < heapSize && array->at(i).key < array->at(l).key)
		largest = l;
	else
		largest = i;
	if (r < heapSize && array->at(largest).key < array->at(r).key)
		largest = r;

	if (largest != i)
	{
		ptrWithKey<T> buffer = array->at(i);
		array->at(i) = array->at(largest);
		array->at(largest) = buffer;
		max_heapify_g<T>(array, largest, heapSize);
	}
}

template<class T>
inline void build_max_heap_g(std::vector<ptrWithKey<T>>* array, size_t arraySize)
{
	for (int i = up_round((float)arraySize / 2) - 1; i >= 0; i--)
		max_heapify_g<T>(array, i, arraySize);
}

template<class T>
inline void heap_sort_g(std::vector<ptrWithKey<T>>* array, size_t arraySize)
{
	build_max_heap_g<T>(array, arraySize);
	size_t heapSize = arraySize;
	ptrWithKey<T> valBuffer = {};

	while (heapSize > 1)
	{
		valBuffer = array->at(heapSize - 1);
		array->at(heapSize - 1) = array->at(0);
		array->at(0) = valBuffer;
		heapSize--;
		max_heapify_g<T>(array, 0, heapSize);
	}
}


