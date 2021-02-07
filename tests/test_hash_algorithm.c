#include <unity.h>

#include "private/common.h"

#define _PFX fmap_test
#define TL_K int
#include "private/hash_algorithm.h"


void setUp(void)
{}

void tearDown(void)
{}


void test_null_term_fnv1a(void)
{
	TEST_ASSERT(0x07f89207b4ba08a4u == tlhash_ntfnv1a("10"));
	TEST_ASSERT(0x779a65e7023cd2e7u == tlhash_ntfnv1a("hello world"));
	TEST_ASSERT(0x50d090ef4acbcc21u == tlhash_ntfnv1a("tset"));
	TEST_ASSERT(0x212fe4f34cebe1b5u == tlhash_ntfnv1a("tsettset"));
}

void test_typed_fnv1a(void)
{
	int key = 1952805748;	//tset
	size_t val = fmap_test_fnv1a(key);
	TEST_ASSERT(0x50d090ef4acbcc21u == val);
}


struct point
{
	int x;
	int y;
};

#undef _PFX
#undef TL_K
#define _PFX fmap_test_point
#define TL_K struct point

#include "private/hash_algorithm.h"

void test_struct_fnv1a(void)
{
	struct point p;
	p.x = 1952805748;	//tset
	p.y = 1952805748;	//tset
	TEST_ASSERT(0x212fe4f34cebe1b5u == fmap_test_point_fnv1a(p));
}


int main(void)
{
	UNITY_BEGIN();

	RUN_TEST(test_null_term_fnv1a);
	RUN_TEST(test_typed_fnv1a);
	RUN_TEST(test_struct_fnv1a);

	return UNITY_END();
}