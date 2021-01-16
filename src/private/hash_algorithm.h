#ifndef _PFX
#error "hash_algorithm.h requires _PFX defined to name the typed function"
#endif

#ifndef TL_K
#error "hash_algorithm.h requires TL_K defined for the tpyed fnv1a function input"
#endif

#ifndef TLSYMBOL
#error "TLSYMBOL(pfx,name) is required to generate the function names (include common.h)"
#endif

#ifndef TL_SIZE_T_BYTES
#error "TL_SIZE_T_BYTES is required and must be 8 or 16 (include common.h)"
#endif

#if (TL_SIZE_T_BYTES == 8)
#define TLHASH_FNV1A_OFFSET 0x811c9dc5
#define TLHASH_FNV1A_PRIME 0x01000193
#elif (TL_SIZE_T_BYTES == 16)
#define TLHASH_FNV1A_OFFSET 0xcbf29ce484222325
#define TLHASH_FNV1A_PRIME 0x00000100000001b3
#endif


#ifndef TEMPLATE_LIB_NULL_TERMINATED_FNV1A
#define TEMPLATE_LIB_NULL_TERMINATED_FNV1A

static inline size_t
tlhash_ntfnv1a(void* key)
{
	unsigned char* data = key;
	size_t hash = TLHASH_FNV1A_OFFSET;

	while (*data != 0) {
		hash = (*data ^ hash) * TLHASH_FNV1A_PRIME;
		data += 1;
	}
	return hash;
}

#endif

static inline size_t
TLSYMBOL(_PFX,fnv1a)(TL_K key)
{
	unsigned char* data = (unsigned char*)&key;
	size_t hash = TLHASH_FNV1A_OFFSET;
	size_t sz = sizeof(TL_K);

	while(sz-- != 0) {
		hash = (*data ^ hash) * TLHASH_FNV1A_PRIME;
		data += 1;
	}
	return hash;
}

#undef TLHASH_FNV1A_PRIME
#undef TLHASH_FNV1A_OFFSET
