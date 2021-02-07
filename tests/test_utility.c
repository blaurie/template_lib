#include <unity.h>

#include "stdlib.h"
#include "private/utility.h"


void setUp(void)
{}

void tearDown(void)
{}


void test_util_npot(void)
{
	TEST_ASSERT_EQUAL_size_t(2u, tl_util_npot(0));
	TEST_ASSERT_EQUAL_size_t(2u, tl_util_npot(1));
	TEST_ASSERT_EQUAL_size_t(2u, tl_util_npot(2));
	TEST_ASSERT_EQUAL_size_t(4u, tl_util_npot(3));
	TEST_ASSERT_EQUAL_size_t(32u, tl_util_npot(20));
	TEST_ASSERT_EQUAL_size_t(1024u, tl_util_npot(1000));
	TEST_ASSERT_EQUAL_size_t(2048u, tl_util_npot(1025));
	TEST_ASSERT_EQUAL_size_t(33554432u, tl_util_npot(32000000));
	TEST_ASSERT_EQUAL_size_t(33554432u, tl_util_npot(33554431u));
}

void test_util_log2n(void)
{
	TEST_ASSERT_EQUAL_size_t(2, tl_util_log2n(4));
	TEST_ASSERT_EQUAL_size_t(2, tl_util_log2n(6));
	TEST_ASSERT_EQUAL_size_t(3, tl_util_log2n(8));
	TEST_ASSERT_EQUAL_size_t(5, tl_util_log2n(32));
	TEST_ASSERT_EQUAL_size_t(25, tl_util_log2n(33554432u));
	TEST_ASSERT_EQUAL_size_t(24, tl_util_log2n(33554431u));
}




int main(void)
{
	UNITY_BEGIN();

	RUN_TEST(test_util_npot);
	RUN_TEST(test_util_log2n);

	return UNITY_END();
}