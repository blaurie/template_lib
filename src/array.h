/**
 * array is an automatically growing array.
 *
 * Options:
 * Define TL_NO_ZERO_MEM to prevent zeroing of memory when not necessary for array to function.
 *
 * Note:
 * All defined are consumed by array.h and will need to be redefined if including again, or including another template.
 *
 * Examples:
 *
 * ---------- Example with a primitive type:
 * #define TL_T int
 * #include <array.h>
 *
 *
 * ---------- Example with a pointer:
 * The notable difference here is that TL_NAME must be defined in order for symbols to successfully generate.
 *
 * #define TL_T char*
 * #define TL_NAME char
 * #include <array.h>
 *
 *
 * ---------- Example with a struct without a typedef:
 * //Keep in mind when using a struct that the compiler must know the definition of the struct before including
 * array.h. A forward declare will not work (because the compiler must know how large the struct is!)
 * struct point
 * {
 * 	int x;
 * 	int y;
 * };
 *
 * #define TL_T struct point
 * #define TL_NAME point
 * #include <array.h>
 */


#include "common.h"

#ifndef TL_T
#error "TL_T macro is required for array.h. Define to a type of choice"
#endif

/**
 * note that this will not work with structs or enums unless they're typedef'd. Hence why this
 * is optionally provided
 */
#ifndef TL_NAME
#define TL_NAME TL_T
#endif

#define TLARRAY_DEFAULT_CAPACITY 20
#define TLARRAY_DEFAULT_GROW_FACTOR 2.0f


#define _PFX TLSYMBOL(array, TL_NAME)

struct _PFX
{
	size_t size;
	size_t capacity;
	float grow_factor;
	TL_T* data;
};

/**
 * array_<TL_NAME>_init_all
 * Initializer that provides user capability to set properties.
 *
 * @param a The array_<TL_NAME> to initialize.
 * @param capacity The initial capacity.
 * @param grow_factor The multiplier that the backing array will grow by upon reaching capacity.
 * @return
 * 	TL_ERR_MEM on failure to allocate memory
 * 	TLOK on initialization success
 */
static inline enum tl_status
TLSYMBOL(_PFX, init_all)(struct _PFX* a, const size_t capacity, const size_t grow_factor)
{
	assert(a != NULL);
	assert(capacity > 1);
	assert(grow_factor > 1.0f);
	TL_T* tmp = tlmalloc(capacity * sizeof(TL_T));
	if (!tmp) return TL_ERR_MEM;

	a->capacity = capacity;
	a->size = 0;
	a->grow_factor = grow_factor;
	a->data = tmp;
	return TLOK;
}


/**
 * array_<TL_NAME>_init
 * Initialize array with default values.
 *
 * NOTES:
 * Default capacity is 20
 * Default grow_factor is 2.0
 *
 * @param a The array_<TL_NAME> to initialize
 * @return
 * 	TL_ERR_MEM on failure to allocate memory
 * 	TLOK on initialization success
 */
static inline enum tl_status
TLSYMBOL(_PFX, init)(struct _PFX* a)
{
	return TLSYMBOL(_PFX, init_all)(a, TLARRAY_DEFAULT_CAPACITY, TLARRAY_DEFAULT_GROW_FACTOR);
}


/**
 * array_<TL_NAME>_deinit
 * Deinitializes the array_<TL_NAME>. The backing array will be freed.
 *
 * OPTIONS:
 * -Define TL_NO_ZERO_MEM to stop overwrite of used memory to 0
 *
 * NOTES:
 * -Elements in the array must be freed by the user if necessary.
 *
 * @param a The array_<TL_NAME> to deinitialize.
 */
static inline void
TLSYMBOL(_PFX, deinit)(struct _PFX* a)
{
	assert(a != NULL);
	assert(a->data != NULL);
#ifndef TL_NO_ZERO_MEM
	tlmemset(a->data, TL_INIT_VAL, a->size * sizeof(TL_T));
	a->capacity = 0;
	a->size = 0;
	a->grow_factor = 0.0f;
#endif
	tlfree(a->data);
	a->data = NULL;
}


/**
 * array_<TL_NAME>_delete
 * Deinitializes and frees a heap allocated array_<TL_NAME> created with array_<TL_NAME>_new[_all](), then sets the pointer
 * to NULL to prevent reuse.
 *
 * OPTIONS:
 * -Define TL_NO_ZERO_MEM to stop overwrite of used memory to 0
 *
 * NOTES:
 * -Elements in the array must be freed by the user if necessary.
 *
 * @param a Pointer to the pointer to array_<TL_NAME> to delete
 */
static inline void
TLSYMBOL(_PFX, delete)(struct _PFX** a)
{
	assert(a != NULL);
	assert(*a != NULL);
	TLSYMBOL(_PFX, deinit)(*a);
#ifndef TL_NO_ZERO_MEM
	tlmemset(*a, TL_INIT_VAL, 1 * sizeof(struct _PFX));
#endif
	tlfree(*a);
	*a = NULL;
}


/**
 * array_<TL_NAME>_new_all
 * Create and initialize a heap allocated array_<TL_NAME>.
 *
 * @param capacity The initial capacity of the backing array.
 * @param grow_factor The multiplier that the backing array will grow by upon reaching capacity.
 * @return
 * 		NULL if allocation or initialization fails.
 * 		Pointer to the heap allocated array on success.
 */
static inline struct _PFX*
TLSYMBOL(_PFX, new_all)(const size_t capacity, const float grow_factor)
{
	assert(capacity > 1);
	assert(grow_factor > 1.0f);
	struct _PFX* ptr = tlmalloc(1 * sizeof(struct _PFX));
	if (!ptr) return NULL;

	if (TLSYMBOL(_PFX, init_all)(ptr, capacity, grow_factor) != TLOK) {
		tlfree(ptr);
		return NULL;
	}
	return ptr;
}


/**
 * array_<TL_NAME>_new
 * Create and initialize a heap allocated array_<TL_NAME> using default values.
 *
 * NOTES:
 * Default capacity is 20
 * Default grow_factor is 2.0
 *
 * @return
 * 		NULL if allocation or initialization fails.
 * 		Pointer to the heap allocated array on success.
 */
static inline struct _PFX*
TLSYMBOL(_PFX, new)(void)
{
	return TLSYMBOL(_PFX, new_all)(TLARRAY_DEFAULT_CAPACITY, TLARRAY_DEFAULT_GROW_FACTOR);
}


/**
 * array_<TL_NAME>_grow
 * This function should normally not be called by a user.
 *
 * Grow the backing array. The new capacity will be (a->capacity * a->grow_factor). If the grow factor is not enough
 * to cause a growth, then 10 will be added to the capacity instead.
 *
 * OPTIONS:
 * -Define TL_NO_ZERO_MEM to stop initializing of the new portion of the backing array to 0.
 *
 * @param a The array to resize the backing memory allocation.
 * @return
 * 	TL_ERR_MEM if there was a failure growing the array for any reason
 * 	TLOK on success
 */
static inline enum tl_status
TLSYMBOL(_PFX, grow)(struct _PFX* a)
{
	const size_t old_capacity = a->capacity;
	size_t new_capacity = old_capacity * a->grow_factor;

	if (new_capacity == old_capacity) new_capacity += 10;
	if (new_capacity < old_capacity) return TL_ERR_MEM;

	TL_T* tmp = tlrealloc(a->data, new_capacity * sizeof(TL_T));
	if (!tmp) return TL_ERR_MEM;

#ifndef TL_NO_ZERO_MEM
	memset(tmp + old_capacity, TL_INIT_VAL, (new_capacity - old_capacity) * sizeof(TL_T));
#endif
	a->data = tmp;
	a->capacity = new_capacity;
	return TLOK;
}




/**
 * array_<TL_NAME>_append
 * Add an element to the end of the array, or place an element in the arracy at the location a->size. If the backing
 * array is too small to add an element, it will automatically resize based on a->capacity * a->grow_factor.
 *
 * OPTIONS:
 * Define TLCLEAR to ensure that the end of the backing array is zeroed if it grows.
 *
 * @param a The array_<TL_NAME> to add an element to.
 * @param element The element to add to the array.
 * @return
 * 	TL_ERR_MEM if the element could not be added (reallocation failed)
 * 	TLOK on success
 */
static inline enum tl_status
TLSYMBOL(_PFX, append)(struct _PFX* a, TL_T element)
{
	assert(a != NULL);
	assert(a->data != NULL);

	if (a->size == a->capacity) {
		if (TLSYMBOL(_PFX, grow)(a) != TLOK) {
			return TL_ERR_MEM;
		}
	}

	a->data[a->size] = element;
	a->size += 1;
	return TLOK;
}


/**
 * array_<TL_NAME>_push
 * Add an element to the front of the array, moving all other elements over by one slot. If the backing array is too
 * small to add the element, it will automatically be resized based on a->capacity * a->grow_factor.
 *
 * OPTIONS:
 * -Define TL_NO_ZERO_MEM to stop zeroing of the end of the array if it grows
 *
 * @param a The array_<TL_NAME> to push an element to.
 * @param element The element to push
 * @return
 * 	TL_ERR_MEM if the element could not be added (reallocation failed)
 * 	TLOK on success
 */
static inline enum tl_status
TLSYMBOL(_PFX, push)(struct _PFX* a, TL_T element)
{
	assert(a != NULL);
	assert(a->data != NULL);

	if (a->size == a->capacity) {
		if (TLSYMBOL(_PFX, grow)(a) != TLOK) {
			return TL_ERR_MEM;
		}
	}

	TL_T* ptr = a->data;
	tlmemmove(ptr + 1, ptr, a->size * sizeof(TL_T));
	ptr[0] = element;
	a->size += 1;
	return TLOK;
}



/**
 * array_<TL_NAME>_insert
 * Insert an element at a particular spot in the array, moving every subsequent element back one if necessary and
 * growing the array if necessary.
 *
 * NOTE:
 * -Insert past size is blocked by assert(). Runtime safety is not guaranteed.
 *
 * @param a The array to insert an element into
 * @param at The location to insert the element at
 * @param element The element to insert
 * @return
 * 	TL_ERR_MEM if the element could not be inserted (reallocation failed)
 * 	TLOK on success
 */
static inline int
TLSYMBOL(_PFX, insert)(struct _PFX* a, const size_t at, TL_T element)
{
	assert(a != NULL);
	assert(a->data != NULL);
	assert(at <= a->size);

	if (a->size == a->capacity) {
		if (TLSYMBOL(_PFX, grow)(a) != TLOK) {
			return TL_ERR_MEM;
		}
	}

	TL_T* ptr = a->data;
	tlmemmove(ptr + at + 1, ptr + at, (a->size - at) * sizeof(TL_T));
	ptr[at] = element;
	a->size += 1;
	return 1;
}


/**
 * array_<TL_NAME>_replace
 * Replace an element in the array at the specified index with the provided value. If you need to return the element
 * that was replaced, use array_<TL_NAME>_exchange instead.
 *
 * NOTE:
 * -Replace past size is blocked by assert(). Runtime safety is not guaranteed.
 *
 * @param a The array to replace an element for
 * @param at The index of the element to replace. Must be smaller than array->size
 * @param element The element to put in to the array
 */
static inline void
TLSYMBOL(_PFX, replace)(struct _PFX* a, const size_t at, TL_T element)
{
	assert(a != NULL);
	assert(a->data != NULL);
	assert(at < a->size);

	a->data[at] = element;
}


/**
 * array_<TL_NAME>_exchange
 * Replace an element in the array at the specified index with the provided value and then return the old value.
 * If you do not need the old value, you should use array_<TLSFX>_replace instead.
 *
 * NOTE:
 * -Exchange past size is blocked by assert(). Runtime safety is not guaranteed.
 *
 * @param a The array to replace an element for
 * @param at The index of the element to replace. Must be smaller than array->size
 * @param element The element to put in to the array
 */
static inline TL_T
TLSYMBOL(_PFX, exchange)(struct _PFX* a, const size_t at, TL_T element)
{
	assert(a != NULL);
	assert(a->data != NULL);
	assert(at < a->size);

	TL_T ret = a->data[at];
	a->data[at] = element;
	return ret;
}


/**
 * array_<TL_NAME>_get
 * Return a copy of the element at the specified index.
 *
 * NOTE:
 * -get past size is blocked by assert(). Runtime safety is not guaranteed.
 *
 * @param a The array to retrieve the element from
 * @param at The index of the desired element
 * @return The element at the desired index
 */
static inline TL_T
TLSYMBOL(_PFX, get)(struct _PFX* a, const size_t at)
{
	assert(a != NULL);
	assert(a->data != NULL);
	assert(at < a->size);

	return a->data[at];
}


/**
 * array_<TL_NAME>_erase
 * Erase an element at the specified index, then move each subsequent element forward by one index. If you need to have
 * the removed element, use array_<TL_NAME>_remove instead.
 *
 * OPTIONS:
 * -Define TL_NO_ZERO_MEM to stop setting the value at the end of the array to 0
 *
 * NOTE:
 * -Erase past size is blocked by assert(). Runtime safety is not guaranteed.
 *
 * @param a The array to remove an element from
 * @param at The index of the element to remove
 */
static inline void
TLSYMBOL(_PFX, erase)(struct _PFX* a, const size_t at)
{
	assert(a != NULL);
	assert(a->data != NULL);
	assert(at < a->size);

	TL_T* dat = a->data;
	tlmemmove(dat + at, dat + at + 1, (a->size - at - 1) * sizeof(TL_T));
	a->size -= 1;
#ifndef TL_NO_ZERO_MEM
	tlmemset(dat + a->size, TL_INIT_VAL, 1 * sizeof(TL_T));
#endif
}


/**
 * array_<TL_NAME>_remove
 * Remove an element at the specified index, then move each subsequent element forward by one index. The removed element
 * is returned. If you do not need the removed element, use array_<TL_NAME>_erase instead.
 *
 * OPTIONS:
 * -Define TL_NO_ZERO_MEM to stop zeroing the newly empty end of the array upon success.
 *
 * NOTE:
 * -Remove past size is blocked by assert(). Runtime safety is not guaranteed.
 *
 * @param a The array to remove an element from
 * @param at The index of the element to remove
 */
static inline TL_T
TLSYMBOL(_PFX, remove)(struct _PFX* a, const size_t at)
{
	assert(a != NULL);
	assert(a->data != NULL);
	assert(at < a->size);

	TL_T* dat = a->data;
	TL_T element = dat[at];
	tlmemmove(dat + at, dat + at + 1, (a->size - at - 1) * sizeof(TL_T));
	a->size -= 1;
#ifndef TL_NO_ZERO_MEM
	tlmemset(dat + a->size, TL_INIT_VAL, 1 * sizeof(TL_T));
#endif
	return element;
}


/**
 * array_<TL_NAME>_shrink_to_fit
 * Shrink the backing array capacity to be only large enough to hold the current elements.
 *
 * @param a The array to shrink
 * @return
 * 	TL_ERR_MEM if the realloc call fails
 * 	TLOK if the shrink was a success
 */
static inline enum tl_status
TLSYMBOL(_PFX, shrink_to_fit)(struct _PFX* a)
{
	assert(a != NULL);
	assert(a->data != NULL);
	assert(a->size > 0);
	TL_T *ptr = tlrealloc(a->data, a->size * sizeof(TL_T));
	if (!ptr) return TL_ERR_MEM;

	a->data = ptr;
	a->capacity = a->size;
	return TLOK;
}


/**
 * array_<TL_NAME>_ensure_capacity
 * Ensure that the backing array has at least enough space to host a given capacities worth of elements.
 *
 * OPTIONS:
 * -Define TL_NO_ZERO_MEM to stop initializing the newly acquired memory to 0
 *
 * @param a The array to check the capacity against
 * @param capacity The capacity to ensure the array can hold
 * @return
 * 	TL_ERR_MEM if the reallocation fails
 * 	TLOK if the array can hold the given capacity
 */
static inline enum tl_status
TLSYMBOL(_PFX, ensure_capacity)(struct _PFX* a, size_t capacity)
{
	assert(a != NULL);
	assert(a->data != NULL);
	if (capacity <= a->capacity) {
		return 1;
	}
	TL_T* ptr = tlrealloc(a->data, capacity * sizeof(TL_T));
	if (!ptr) return TL_ERR_MEM;

#ifndef TL_NO_ZERO_MEM
	tlmemset(ptr + a->capacity, TL_INIT_VAL, (capacity - a->capacity) * sizeof(TL_T));
#endif
	a->data = ptr;
	a->capacity = capacity;
	return TLOK;
}


/**
 * array_<TL_NAME>_clear
 * Set the size of this array_<TL_NAME> to 0.
 *
 * OPTIONS:
 * -Define TL_NO_ZERO_MEM to stop initializing the backing array to 0 during clear.
 *
 * NOTES:
 * -free is not called on the elements, it is up to the user if necessary.
 *
 * @param a The array_<TL_NAME> to clear
 */
static inline void
TLSYMBOL(_PFX, clear)(struct _PFX* a)
{
	assert(a != NULL);
	assert(a->data != NULL);
#ifndef TL_NO_ZERO_MEM
	tlmemset(a->data, TL_INIT_VAL, a->size * sizeof(TL_T));
#endif
	a->size = 0;
}



#undef _PFX
#undef TLARRAY_DEFAULT_CAPACITY
#undef TLARRAY_DEFAULT_GROW_FACTOR
#undef TL_NO_ZERO_MEM
#undef TL_NAME
#undef TL_T
