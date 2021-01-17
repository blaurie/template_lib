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
#define fmap_key_equalsfn(left,right) (left) == (right)
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
#define TL_FMAP_DEFAULT_LOAD_FACTOR 0.7f



/**
 * fmap_<TL_NAME>_node
 * flatmap node containing a key, value pair.
 */
struct TLSYMBOL(_PFX,node)
{
	TL_K key;
	TL_V value;
};

struct _PFX
{
	size_t num_buckets;
	size_t bucket_max;
	size_t capacity;
	size_t load_max;
	size_t size;
	size_t slot_mask;
	struct TLSYMBOL(_PFX,node)* nodes;
	enum tl_map_slot_state* info;
	float load_factor;
};




//todo: init
//todo: init_all
//todo: deinit
//todo: new
//todo: new_all
//todo: delete

//todo: probe for open
//todo: probe for key

//todo: rehash
//todo: grow

//todo: <add> a new key value pair
//todo: <insert> a key value pair (or replace if the key exists)
//todo: <erase> a key value pair and don't return the value
//todo: <remove> a key value pair and return the value
//todo: <clear> the whole map
//todo: <get> a value for a provided key
//todo: <try_get> a value for a provided key, returning a status






#undef TL_FMAP_DEFAULT_LOAD_FACTOR
#undef TL_FMAP_DEFAULT_BUCKET_COUNT
#undef fmap_hashfn
#undef _PFX
#undef TL_NAME
#undef fmap_key_equalsfn
#undef TL_V
#undef TL_K
