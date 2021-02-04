/**
 * Flatmap is a "power of 2" implementation, which means that growth happens exponentially.
 *
 * This implementation is flat, and backed by a contiguous array which allows for log2n amount of space
 * per bucket. Buckets are probed linearly. Keeping this in mind, consider that a grow must occur both
 * when a bucket is over capacity, as well as when the load factor is hit. As a result, if you provide
 * a hash algorithm, you must test its efficacy for providing uniform distribution.
 *
 * The total memory usage of the backing array at any given time is (num_buckets * log2n(num_buckets)).
 *
 * This can be represented as below, where each bucket is separated by a pipe | and each slot is represented
 * by a (n).
 *   Bucket 1     Bucket 2
 * |(1)(2)(3)(4)|(1)(2)(3)(4)|....
 * Slot 1 of a given bucket is the fastest to acquire.
 *
 * We also provide 2 hashing functions by default:
 * tlhash_ntfnv1a(key)		- Hash till reaching a null terminator value (good for c strings)
 * fmap_<TL_NAME>_fnv1a(key)	- Hash for the sizeof(TL_K)
 *
 * If you need any other hashing behavior, it is up to the user to provide it and define fmap_hashfn(key).
 *
 * Note:
 * -Must define TL_K to set the key type
 * -Must define TL_V to set the value type
 *
 * -All user #define are consumed by the #include and must be redefined again to include again!
 *
 * Options:
 * -Define fmap_key_equalsfn(left,right) to override the key equality test behavior
 * 	-Default behavior is a simple equality operator
 * -Define fmap_hashfn(key) to provide your own hashing function (must accept key type and return size_t)
 * 	-Default is provided fmap_<TL_NAME>_fnv1a
 * -Define TL_NAME to set the provided name
 * 	-Default is to concatenate the TL_K and TL_V values
 * -Define TL_NO_ZERO_MEM to stop zeroing stop the zeroing of memory in non-critical code
 * -Define TL_KEY_IS_NT to use the provided tlhash_ntfnv1a(key) instead of fmap_<TL_NAME>_fnv1a(key)
 *
 *
 * Examples:
 * todo: examples for how to include!
 */

#ifndef TL_K
#error "TL_K not defined for flatmap_h"
#endif

#ifndef TL_V
#error "TL_V not define dfor flatmap.h"
#endif

#include "private/common.h"
#include "private/utility.h"
#include "private/map_slot_state.h"

#ifndef TL_NAME
#define TL_NAME TLCONCAT(TL_K,TL_V)
#endif

#define _PFX TLSYMBOL(fmap,TL_NAME)

/**
 * Enable user provided key equality function
 */
#ifndef fmap_key_equalsfn
#define fmap_key_equalsfn(left, right) (left) == (right)
#endif

/**
 * Enable user provided hash function
 */
#ifndef fmap_hashfn

#include "private/hash_algorithm.h"

#  ifdef TL_KEY_IS_NT
#    define fmap_hashfn(key) tlhash_ntfnv1a(key)
#  else
#    define fmap_hashfn(key) TLSYMBOL(_PFX,fnv1a)(key)
#  endif
#endif

/**
 * section for defaut values
 */
//TL_FMAP_DEFAULT_BUCKET_COUNT must be power of 2
#define TL_FMAP_DEFAULT_BUCKET_COUNT 8u
#define TL_FMAP_DEFAULT_LOAD_FACTOR 70u


/**
 * fmap_<TL_NAME>_node
 * flatmap node containing a key, value pair.
 */
struct TLSYMBOL(_PFX, node)
{
	TL_K key;
	TL_V value;
};

/**
 * num_buckets - (private) The number of the buckets
 * bucket_max  - (private) Max elements in each bucket
 * capacity    - (private) The total elements the map can hold (num_buckets * bucket_max)
 * load_max    - (private) The total elements before the map should grow
 * size        - (public) The number of elements current in the map
 * slot_mask   - (private) The mask used to transform a hash to a bucket index
 * nodes       - (private) The elements
 * info        - (private) Extra information about each node location
 * load_factor - (private) The fill percentage (0-100) to target before growth
 */
struct _PFX
{
	size_t num_buckets;
	size_t bucket_max;
	size_t capacity;
	size_t load_max;
	size_t size;
	size_t slot_mask;
	struct TLSYMBOL(_PFX, node)* nodes;
	enum tl_map_slot_state* info;
	size_t load_factor;
};


/**
 * fmap_<TL_NAME>_init_all
 * Initialize a fmap_<TL_NAME> struct fields, allowing the user to provide configuration.
 *
 * Note:
 * -That the num_buckets isn't the capacity. Capacity is calculated (num_buckets * (log2n(num_buckets))
 *
 * @param fm
 * @param num_buckets
 * @param load_factor
 * @return
 */
static inline enum tl_status
TLSYMBOL(_PFX, init_all)(struct _PFX* fm, const size_t num_buckets, const size_t load_factor)
{
	assert(fm != NULL);
	assert(num_buckets > 1u);
	assert(load_factor <= 100);

	const size_t buckets = tl_util_npot(num_buckets);
	const size_t bucket_max = tl_util_log2n(buckets);
	const size_t capacity = buckets * bucket_max;
	const size_t factor = (load_factor != 0) ? load_factor : TL_FMAP_DEFAULT_LOAD_FACTOR;

	struct TLSYMBOL(_PFX, node)* nodes = tlcalloc(capacity, sizeof(struct TLSYMBOL(_PFX, node)));
	if (!nodes)
		return TL_ERR_MEM;

	enum tl_map_slot_state* info = tlcalloc(capacity, sizeof(enum tl_map_slot_state));
	if (!info) {
		tlfree(nodes);
		return TL_ERR_MEM;
	}

	fm->num_buckets = buckets;
	fm->bucket_max = bucket_max;
	fm->capacity = capacity;
	fm->load_max = (capacity * factor) / 100u;
	fm->size = 0;
	fm->slot_mask = buckets - 1;
	fm->nodes = nodes;
	fm->info = info;
	fm->load_factor = factor;

	return TLOK;
}

//todo: init
static inline enum tl_status
TLSYMBOL(_PFX, init)(struct _PFX* fm)
{
	return TLSYMBOL(_PFX, init_all)(fm, TL_FMAP_DEFAULT_BUCKET_COUNT, TL_FMAP_DEFAULT_LOAD_FACTOR);
}


//todo: deinit
static inline void
TLSYMBOL(_PFX, deinit)(struct _PFX* fm)
{
	assert(fm != NULL);
	assert(fm->nodes != NULL);
	assert(fm->info != NULL);
#ifndef TL_NO_ZERO_MEM
	tlmemset(fm->nodes, TL_INIT_VAL, fm->capacity * sizeof(struct TLSYMBOL(_PFX, node)));
	tlmemset(fm->info, TL_INIT_VAL, fm->capacity * sizeof(enum tl_map_slot_state));
	fm->size = 0u;
	fm->capacity = 0u;
	fm->bucket_max = 0u;
	fm->load_factor = 0u;
	fm->num_buckets = 0u;
	fm->load_max = 0u;
	fm->slot_mask = 0u;
#endif
	tlfree(fm->nodes);
	tlfree(fm->info);
	fm->nodes = NULL;
	fm->info = NULL;
}


//todo: new_all
static inline struct _PFX*
TLSYMBOL(_PFX, new_all)(const size_t num_buckets, const float load_factor)
{
	assert(num_buckets > 1u);
	assert(load_factor <= 100u);
	struct _PFX* tmp = tlmalloc(sizeof(struct _PFX));
	if (!tmp)
		return NULL;

	if (TLSYMBOL(_PFX, init_all)(tmp, num_buckets, load_factor) != TLOK) {
		tlfree(tmp);
		return NULL;
	}

	return tmp;
}


//todo: new
static inline struct _PFX*
TLSYMBOL(_PFX, new)()
{
	return TLSYMBOL(_PFX, new_all)(TL_FMAP_DEFAULT_BUCKET_COUNT, TL_FMAP_DEFAULT_LOAD_FACTOR);
}


//todo: delete
static inline void
TLSYMBOL(_PFX, delete)(struct _PFX** fm)
{
	assert(*fm != NULL);

	/* this if "shouldn't" be necessary unless user is a dumb dumb. deinit asserts anyway. */
	//if ((*fm)->nodes != NULL)
	TLSYMBOL(_PFX, deinit)(*fm);

#ifndef TL_NO_ZERO_MEM
	tlmemset(*fm, TL_INIT_VAL, sizeof(struct _PFX));
#endif
	tlfree(*fm);
	*fm = NULL;
}


/**
 * probe_open is for internal use only
 */
static inline size_t
TLSYMBOL(_PFX, probe_open)(const enum tl_map_slot_state* info, const size_t bucket_index, const size_t bucket_capacity)
{
	size_t slot;

	for (slot = 0; slot < bucket_capacity; slot++) {
		/**
		 * When we are zeroing memory, the info bucket will always be 0, plus we move the elements.
		 * Otherwise, this bucket may have a couple statuses that mean "available".
		 */
#ifndef TL_NO_ZERO_MEM
		if (info[bucket_index + slot] == TL_MAPSS_EMPTY) break;
#else
		if (info[bucket_index + slot] == TL_MAPSS_EMPTY || info[bucket_index + slot] == TL_MAPSS_DELETED) break;
#endif
	}

	return slot;
}

/**
 * probe_key is for internal use only
 */
static inline enum tl_status
TLSYMBOL(_PFX, probe_key)(struct TLSYMBOL(_PFX, node)* nodes, const enum tl_map_slot_state* info,
	const size_t bucket_index, const size_t bucket_capacity, TL_K key, size_t* out_slot)
{
	size_t slot;
	size_t delt_slot = bucket_capacity;

	for (slot = 0; slot < bucket_capacity; slot++) {
		if (info[bucket_index + slot] == TL_MAPSS_EMPTY) {
			*out_slot = slot;
			return TL_ENF;
		}

#ifdef TL_NO_ZERO_MEM
		if (info[bucket_index + slot] == TL_MAPSS_DELETED) {
			if (delt_slot != bucket_capacity) delt_slot = slot;
			continue;
		}
#endif
		if (fmap_key_equalsfn(nodes[bucket_index + slot].key, key)) {
			*out_slot = slot;
			return TLOK;
		}
	}

	if (delt_slot != bucket_capacity) slot = delt_slot;

	*out_slot = slot;
	return (slot < bucket_capacity) ? TL_ENF : TL_OOB;
}

/**
 * rehash is for internal use only
 * note - rehash requires new_nodes to be large enough!
 */
static inline void
TLSYMBOL(_PFX, rehash)(struct TLSYMBOL(_PFX, node)* old_nodes, const enum tl_map_slot_state* old_info, const size_t old_capacity,
	struct TLSYMBOL(_PFX, node)* new_nodes, enum tl_map_slot_state* new_info, const size_t new_bucket_max, const size_t new_mask)
{
	for (size_t slot = 0; slot < old_capacity; slot++) {
		if (old_info[slot] == TL_MAPSS_OCCUPIED || old_info[slot] == TL_MAPSS_COLLIDED) {
			const size_t bucket = (fmap_hashfn(old_nodes[slot].key) & new_mask) * new_bucket_max;
			const size_t new_slot = TLSYMBOL(_PFX, probe_open)(new_info, bucket, new_bucket_max);
			const size_t pos = bucket + new_slot;

			new_nodes[pos] = old_nodes[slot];
			new_info[pos] = (new_slot == 0) ? TL_MAPSS_OCCUPIED : TL_MAPSS_COLLIDED;
		}
	}
}


//todo: grow
static inline enum tl_status
TLSYMBOL(_PFX, grow)(struct _PFX* fm)
{
	assert(fm != NULL);
	assert(fm->nodes != NULL);
	assert(fm->info != NULL);

	const size_t new_buckets = fm->num_buckets << 1;
	const size_t new_mask = new_buckets - 1;
	const size_t new_bucket_capacity = tl_util_log2n(new_buckets);
	const size_t new_capacity = new_buckets * new_bucket_capacity;

	struct TLSYMBOL(_PFX, node)* new_nodes = tlcalloc(new_capacity, sizeof(struct TLSYMBOL(_PFX, node)));
	if (!new_nodes)
		return TL_ERR_MEM;

	enum tl_map_slot_state* new_info = tlcalloc(new_capacity, sizeof(enum tl_map_slot_state));
	if (!new_info) {
		tlfree(new_nodes);
		return TL_ERR_MEM;
	}

	TLSYMBOL(_PFX, rehash)(fm->nodes, fm->info, fm->capacity, new_nodes, new_info, new_bucket_capacity, new_mask);

#ifndef TL_NO_ZERO_MEM
	tlmemset(fm->nodes, TL_INIT_VAL, fm->capacity * sizeof(struct TLSYMBOL(_PFX, node)));
	tlmemset(fm->info, TL_INIT_VAL, fm->capacity * sizeof(enum tl_map_slot_state));
#endif
	tlfree(fm->nodes);
	tlfree(fm->info);
	fm->nodes = new_nodes;
	fm->info = new_info;
	fm->num_buckets = new_buckets;
	fm->bucket_max = new_bucket_capacity;
	fm->slot_mask = new_mask;
	fm->capacity = new_capacity;
	fm->load_max = (new_capacity * fm->load_factor) / 100;
	return TLOK;
}


//todo: <add> a new key value pair
static inline enum tl_status
TLSYMBOL(_PFX, add)(struct _PFX* fm, TL_K key, TL_V value)
{
	assert(fm != NULL);
	assert(fm->nodes != NULL);
	assert(fm->info != NULL);

	if (fm->size >= fm->load_max) {
		if (TLSYMBOL(_PFX, grow)(fm) != TLOK)
			return TL_ERR_MEM;
	}

	const size_t hash = fmap_hashfn(key);
	size_t slot;
	size_t slot_index;

	RETRY_ADD:
	slot = hash & fm->slot_mask;
	slot *= fm->bucket_max;
	slot_index = 0;

	switch (TLSYMBOL(_PFX, probe_key)(fm->nodes, fm->info, slot, fm->bucket_max, key, &slot_index)) {
	case TL_ENF:                /* What we want for this function! */
		fm->nodes[slot + slot_index].key = key;
		fm->nodes[slot + slot_index].value = value;
		fm->info[slot + slot_index] = (slot_index == 0) ? TL_MAPSS_OCCUPIED : TL_MAPSS_COLLIDED;
		fm->size++;
		return TLOK;
	case TLOK:                /* Actually bad for this function! */
		return TL_EAE;
	case TL_OOB:
		if (TLSYMBOL(_PFX, grow)(fm) != TLOK)
			return TL_ERR_MEM;

		goto RETRY_ADD;
	default:
		return TL_ERROR;
	}
}


//todo: <get> a value for a provided key
static inline TL_V
TLSYMBOL(_PFX,get)(struct _PFX* fm, TL_K key)
{
	assert(fm != NULL);
	assert(fm->nodes != NULL);
	assert(fm->info != NULL);

	const size_t hash = TLSYMBOL(_PFX,fnv1a)(key);
	const size_t bucket = hash & fm->slot_mask;
	const size_t slot = bucket * fm->bucket_max;
	size_t slot_idx = 0;

	if (TLSYMBOL(_PFX,probe_key)(fm->nodes, fm->info, slot, fm->bucket_max, key, &slot_idx) != TLOK) {
		TL_V value;
		tlmemset(&value, TL_INIT_VAL, sizeof(TL_V));
		return value;
	}

	return fm->nodes[slot + slot_idx].value;
}


//todo: <try_get> a value for a provided key, returning a status
static inline enum tl_status
TLSYMBOL(_PFX,try_get)(struct _PFX* fm, TL_K key, TL_V* out_value)
{
	assert(fm != NULL);
	assert(fm->nodes != NULL);
	assert(fm->info != NULL);

	const size_t hash = TLSYMBOL(_PFX,fnv1a)(key);
	const size_t bucket = hash & fm->slot_mask;
	const size_t slot = bucket * fm->bucket_max;
	size_t slot_idx = 0;

	if (TLSYMBOL(_PFX,probe_key)(fm->nodes, fm->info, slot, fm->bucket_max, key, &slot_idx) != TLOK) {
		return TL_ENF;
	}

	*out_value = fm->nodes[slot + slot_idx].value;
	return TLOK;
}


//todo: <insert> a key value pair (or replace if the key exists)
static inline enum tl_status
TLSYMBOL(_PFX,insert)(struct _PFX* fm, TL_K key, TL_V value)
{
	assert(fm != NULL);
	assert(fm->nodes != NULL);
	assert(fm->info != NULL);

	if (fm->size >= fm->load_max) {
		if (TLSYMBOL(_PFX, grow)(fm) != TLOK)
			return TL_ERR_MEM;
	}

	const size_t hash = fmap_hashfn(key);
	size_t slot;
	size_t slot_index;

	RETRY_ADD:
	slot = hash & fm->slot_mask;
	slot *= fm->bucket_max;
	slot_index = 0;

	switch (TLSYMBOL(_PFX, probe_key)(fm->nodes, fm->info, slot, fm->bucket_max, key, &slot_index)) {
	case TL_ENF:
		fm->size++;
	case TLOK:
		fm->nodes[slot + slot_index].key = key;
		fm->nodes[slot + slot_index].value = value;
		fm->info[slot + slot_index] = (slot_index == 0) ? TL_MAPSS_OCCUPIED : TL_MAPSS_COLLIDED;
		return TLOK;
	case TL_OOB:
		if (TLSYMBOL(_PFX, grow)(fm) != TLOK)
			return TL_ERR_MEM;

		goto RETRY_ADD;
	default:
		return TL_ERROR;
	}
}




//todo: <erase> a key value pair and don't return the value
//todo: <remove> a key value pair and return the value
//todo: <clear> the whole map







#undef TL_FMAP_DEFAULT_LOAD_FACTOR
#undef TL_FMAP_DEFAULT_BUCKET_COUNT
#undef fmap_hashfn
#undef _PFX
#undef TL_NAME
#undef fmap_key_equalsfn
#undef TL_NO_ZERO_MEM
#undef TL_V
#undef TL_K
