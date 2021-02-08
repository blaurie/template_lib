/**
 * Map is a "power of 2" implementation, meaning growth happens exponentially.
 *
 * This implementation has flat nodes and uses linear probing + robinhooding to try to balance performance
 * and space. Incidentally, this means that each node has overhead in the form of tracking the distance from its
 * favourite position.
 *
 * This map will also track a maximum probe sequence length from desired to ensure we aren't stuck probing for too long
 * on insert. The max psl an element is allowed to be from its favourite position is log2n(capacity) * 2.
 * If the max_distance is hit, a grow will be forced. Because we have a max probe length, we also take liesure to pad
 * the end of the array so that we don't need any special probing at the end.
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

struct TLSYMBOL(_PFX, node)
{
	enum tl_map_slot_state state;
	unsigned char psl;
	TL_K key;
	TL_V value;
};

struct _PFX
{
	size_t capacity;
	size_t max_psl;
	size_t size;
	size_t slot_mask;
	size_t load_factor;
	size_t load_max;
	struct TLSYMBOL(_PFX, node)* nodes;
};



//todo: init_all
static inline enum tl_status
TLSYMBOL(_PFX, init_all)(struct _PFX* m, size_t capacity, size_t load_factor)
{
	assert(m != NULL);
	assert(capacity > 1u);
	assert(load_factor >= 100u);

	const size_t cap = tl_util_npot(capacity);
	const size_t factor = (load_factor != 0) ? load_factor : TL_MAP_DEFAULT_LOAD_FACTOR;
	const size_t psl = tl_util_log2n(cap) * 2;

	struct TLSYMBOL(_PFX, node)* nodes = tlcalloc(cap + psl, sizeof(struct TLSYMBOL(_PFX, node)));
	if (!nodes)
		return TL_ERR_MEM;

	m->capacity = cap;
	m->max_psl = psl;
	m->size = 0;
	m->slot_mask = cap - 1;
	m->load_factor = factor;
	m->load_max = (cap * factor) / 100u;
	m->nodes = nodes;
	return TLOK;
}


//todo: init
static inline enum tl_status
TLSYMBOL(_PFX, init)(struct _PFX* m)
{
	return TLSYMBOL(_PFX, init_all)(m, TL_MAP_DEFAULT_CAPACITY, TL_MAP_DEFAULT_LOAD_FACTOR);
}

//todo: deinit
static inline void
TLSYMBOL(_PFX, deinit)(struct _PFX* m)
{
	assert(m != NULL);
	assert(m->nodes != NULL);
	tlmemset(m->nodes, TL_INIT_VAL, (m->capacity + m->max_psl) * sizeof(struct TLSYMBOL(_PFX, node)));
	m->capacity = 0;
	m->max_psl = 0;
	m->size = 0;
	m->slot_mask = 0;
	m->load_factor = 0;
	m->load_max = 0;
	m->nodes = NULL;
}

//todo: new_all
static inline struct _PFX*
TLSYMBOL(_PFX, new_all)(size_t capacity, size_t load_factor)
{
	assert(capacity > 1u);
	assert(load_factor >= 100u);

	struct _PFX* map = tlcalloc(1, sizeof(struct _PFX));
	if (!map)
		return NULL;

	switch (TLSYMBOL(_PFX, init_all)(map, capacity, load_factor)) {
	case TLOK:
		return map;
	default:
		tlfree(map);
		return NULL;
	}
}

//todo: new
static inline struct _PFX*
TLSYMBOL(_PFX, new)()
{
	return TLSYMBOL(_PFX, new_all)(TL_MAP_DEFAULT_CAPACITY, TL_MAP_DEFAULT_LOAD_FACTOR);
}

//todo: delete
static inline void
TLSYMBOL(_PFX, delete)(struct _PFX** m)
{
	assert(m != NULL);
	assert(*m != NULL);
	assert((*m)->nodes != NULL);

	TLSYMBOL(_PFX, deinit)(*m);
	tlmemset(*m, TL_INIT_VAL, sizeof(struct _PFX));
	tlfree(*m);
	*m = NULL;
}

//todo: probe_open
//todo: probe_key


//todo: rehash
static inline void
TLSYMBOL(_PFX, rehash)(struct TLSYMBOL(_PFX, node)* old_nodes, size_t old_capacity,
	struct TLSYMBOL(_PFX, node)* new_nodes, size_t new_distance, size_t new_mask)
{

}


//todo: grow
static inline enum tl_status
TLSYMBOL(_PFX, grow)(struct _PFX* m)
{
	assert(m != NULL);
	assert(m->nodes != NULL);


	return TLOK;
}


//todo: add (and do not replace)
static inline enum tl_status
TLSYMBOL(_PFX, add)(struct _PFX* m, TL_K key, TL_V value)
{
	assert(m != NULL);
	assert(m->nodes != NULL);

	if (m->size == m->load_max) {
		if (TLSYMBOL(_PFX, grow)(m) != TLOK)
			return TL_ERR_MEM;
	}

	const size_t hash = map_hashfn(key);
	size_t slot = hash & m->slot_mask;

	struct TLSYMBOL(_PFX,node) new_node;
	new_node.state = TL_MAPSS_OCCUPIED;
	new_node.key = key;
	new_node.value = value;

	struct TLSYMBOL(_PFX,node) tmp;		/* used for swapping if needed */

	for (new_node.psl = 0; new_node.psl < m->max_psl; new_node.psl++) {
		if (m->nodes[slot + new_node.psl].state == TL_MAPSS_EMPTY) {
			m->nodes[slot + new_node.psl] = new_node;
			break;
		}
		if (map_key_equalsfn(key, m->nodes[slot + new_node.psl].key)) {
			return TL_EAE;
		}
		if (new_node.psl > m->nodes[slot + new_node.psl].psl) {
			tmp = m->nodes[slot + new_node.psl];
			m->nodes[slot + new_node.psl] = new_node;
			new_node = tmp;
		}
	}

	/* we couldn't find a slot */
	if (new_node.psl == m->max_psl) {
		if (TLSYMBOL(_PFX, grow)(m) != TLOK)
			return TL_ERR_MEM;
	}

	return TLOK;

}

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
