#ifndef TEMPLATE_LIB_UTILITY_H
#define TEMPLATE_LIB_UTILITY_H

/**
 * tl_util_npot
 * Returns the next closest power of two for a given unsigned number.
 *
 * @param s The number to calculate the next power of two for
 * @return The calculated power of two number
 */
static inline size_t
tl_util_npot(size_t s)
{
	if (s < 2) return 2;

	size_t runs;
	size_t sz = sizeof(size_t) * 8;
	for (runs = 0; sz > 8; runs++) {
		sz = sz >> 1u;
	}
	runs++;

	s--;
	size_t shift = 1u;
	for (size_t i = 0; i <= runs; i++) {
		s |= s >> shift;
		shift <<= 1u;
	}
	return ++s;
}

/**
 * tl_util_log2n
 * Calculates the log2n of a given unsigned number.
 *
 * @param num The number to calculate the log2n of
 * @return The log2n of the given number
 */
static inline size_t
tl_util_log2n(size_t num)
{
	size_t ret = 0;

	while(num) {
		num = num >> 1u;
		ret++;
	}
	return ret - 1;
}

#endif //TEMPLATE_LIB_UTILITY_H
