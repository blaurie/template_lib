/**
 * Enables user to provide custom allocators
 */
#include <stdlib.h>        /* malloc, calloc, realloc */
#include <string.h>        /* memset, memmove */


#ifndef tlmalloc
#define tlmalloc(size) malloc((size))
#endif
#ifndef tlcalloc
#define tlcalloc(nmemb,size) calloc((nmemb),(size))
#endif
#ifndef tlrealloc
#define tlrealloc(ptr,size) realloc((ptr),(size))
#endif
#ifndef tlfree
#define tlfree(ptr) free(ptr)
#endif
#ifndef tlmemset
#define tlmemset(ptr,value,length) memset((ptr),(value),(length))
#endif
#ifndef tlmemmove
#define tlmemmove(dest,src,length) memmove((dest),(src),(length))
#endif


/** todo: do some more testing to pick the implementation
 * --The macros most like more reliably compile to better code
 * --The static functions provide better type verification
 * --A struct which each container copies has nicer locality to the container
 * 	but is likely quite removed from what guarantees good code being emitted.
 *
#include "tlstatus.h"

typedef void* tlmalloc_fn(size_t size);
typedef void* tlcalloc_fn(size_t nmemb, size_t size);
typedef void* tlrealloc_fn(void* ptr, size_t size);
typedef void tlfree_fn(void* ptr);
typedef void* tlmemset_fn(void* ptr, int value, size_t length);
typedef void* tlmemmove_fn(void* dest, const void* src, size_t length);

static tlmalloc_fn* tlmalloc = &malloc;
static tlcalloc_fn* tlcalloc = &calloc;
static tlrealloc_fn* tlrealloc = &realloc;
static tlfree_fn* tlfree = &free;
static tlmemset_fn* tlmemset = &memset;
static tlmemmove_fn* tlmemmove = &memmove;

static inline enum tl_status
tl_memory_set(tlmalloc_fn* new_malloc,
	      tlcalloc_fn* new_calloc,
	      tlrealloc_fn* new_realloc,
	      tlfree_fn* new_free,
	      tlmemset_fn* new_memset,
	      tlmemmove_fn* new_memmove)
{
	if (new_malloc == NULL) return TL_ERRIN_MMR;
	if (new_calloc == NULL) return TL_ERRIN_MCR;
	if (new_realloc == NULL) return TL_ERRIN_MRR;
	if (new_free == NULL) return TL_ERRIN_MFR;
	if (new_memset == NULL) return TL_ERRIN_MMMR;
	if (new_memmove == NULL) return TL_ERRIN_MMSR;

	tlmalloc = new_malloc;
	tlcalloc = new_calloc;
	tlrealloc = new_realloc;
	tlfree = new_free;
	tlmemset = new_memset;
	tlmemmove = new_memmove;

	return TLOK;
}
 */


