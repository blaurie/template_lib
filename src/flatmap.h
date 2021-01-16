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
 * tl_fnv1a_nt(key)		- Hash till reaching a null terminator value (good for c strings)
 * tl_fnv1a_<TL_NAME>(key)	- Hash for the sizeof(TL_K)
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
 * 	-Default is provided tl_fnv1a_<TL_NAME>
 * -Define TL_NAME to set the provided name
 * 	-Default is to concatenate the TL_K and TL_V values
 * -Define TL_NO_ZERO_MEM to stop zeroing stop the zeroing of memory in non-critical code
 * -Define TL_KEY_IS_NT to use the provided tl_fnv1a_nt(key) instead of tl_fnv1a_<TL_NAME>(key)
 *
 */

#ifndef TL_K
#error "TL_K not defined for flatmap_h"
#endif

#ifndef TL_V
#error "TL_V not define dfor flatmap.h"
#endif

#include "common.h"

#ifndef fmap_key_equalsfn
#define fmap_key_equalsfn(left,right) (left) == (right)
#endif

#ifndef TL_NAME
#define TL_NAME TLCONCAT(TL_K,TL_V)
#endif




#undef TL_NAME
#undef fmap_key_equalsfn
#undef TL_V
#undef TL_K
