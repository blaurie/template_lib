/**
 * Map is a "power of 2" implementation, meaning growth happens exponentially.
 *
 * This implementation has flat nodes and uses quadratic probing + robinhooding to try to balance performance
 * and space. Incidentally, this means that each node has overhead in the form of a tombstone enum, as well
 * as a unsigned char to track the distance from desired.
 *
 * This map will also track a maximum distance from desired to ensure we aren't stuck probing for too long
 * on insert. The max distance an element is allowed to be from its favourite position is log2n(capacity) * 2.
 * If the max_distance is hit, a grow will be forced.
 *
 * I also provide 2 hashing functions by default:
 * tlhash_ntfnv1a(key)		- Hash till reaching a null terminator value (good for c strings)
 * map_<TL_NAME>_fnv1a(key)	- Hash for the sizeof(TL_K)
 *
 * If you need any other hashing behavior, it is up to the user to provide it and define map_hashfn(key).
 *
 * Note:
 * TL_K must be defined to set the key type
 * TL_V must be defined to set the value type
 *
 * All users #defines are consumed by the include of map.h.
 *
 * Options:
 * -Define map_key_equalsfn(left,right) to override the key equality test behavior
 * 	-Default behavior is a simple equality operator
 * -Define map_hashfn(key) to provide your own hashing function (must accept key type and return size_t)
 * 	-Default is provided map_<TL_NAME>_fnv1a
 * -Define TL_NAME to set the provided name
 * 	-Default is to concatenate the TL_K and TL_V values
 * -Define TL_KEY_IS_NT to use the provided tlhash_ntfnv1a(key) instead of map_<TL_NAME>_fnv1a(key)
 *
 */

#ifndef TL_K
#error "TL_K not defined for flatmap_h"
#endif

#ifndef TL_V
#error "TL_V not defined for flatmap.h"
#endif

#include "private/common.h"
#include "private/utility.h"
#include "private/map_slot_state.h"

#ifndef TL_NAME
#define TL_NAME TLCONCAT(TL_K,TL_V)
#endif

#define _PFX TLSYMBOL(map,TL_NAME)


/**
 * Enable user provided key equality function
 */
#ifndef map_key_equalsfn
#define map_key_equalsfn(left, right) (left) == (right)
#endif

/**
 * Enable user provided hash function
 */
#ifndef map_hashfn

#include "private/hash_algorithm.h"

#  ifdef TL_KEY_IS_NT
#    define map_hashfn(key) tlhash_ntfnv1a(key)
#  else
#    define map_hashfn(key) TLSYMBOL(_PFX,fnv1a)(key)
#  endif
#endif

/**
 * section for defaut values
 */
#define TL_MAP_DEFAULT_CAPACITY 16u
#define TL_MAP_DEFAULT_LOAD_FACTOR 70u

struct TLSYMBOL(_PFX,node)
{
	enum tl_map_slot_state state;
	unsigned char distance;
	TL_K key;
	TL_V value;
};

struct _PFX
{
	size_t capacity;
	size_t max_distance;
	size_t size;
	size_t slot_mask;
	size_t load_factor;
	size_t load_max;
	struct TLSYMBOL(_PFX,node)* nodes;
};



//todo: init_all
//todo: init
//todo: deinit
//todo: new_all
//todo: new
//todo: delete
//todo: probe_open
//todo: probe_key
//todo: rehash
//todo: grow
//todo: add (and do not replace)
//todo: get
//todo: try_get
//todo: insert (or replace)
//todo: erase (and do not return value)
//todo: remove (and return value)
//todo: clear








#undef TL_MAP_DEFAULT_LOAD_FACTOR
#undef TL_MAP_DEFAULT_CAPACITY
#undef map_hashfn
#undef map_key_equalsfn
#undef _PFX
#undef TL_NAME
#undef TL_KEY_IS_NT
#undef TL_V
#undef TL_k
