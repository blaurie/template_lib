#include <unity.h>

#include <stdint.h>

#define TL_K int
#define TL_V int
#include "flatmap.h"

void setUp(void)
{}

void tearDown(void)
{}

/**
 * test_generated_node and test_generated_type simply show the fields we expect. Very simple.
 */
void test_generated_node(void)
{
	struct fmap_intint_node node = {0};

	TEST_ASSERT_EQUAL_INT(0, node.key);
	TEST_ASSERT_EQUAL_INT(0, node.value);
}

void test_generated_type(void)
{
	struct fmap_intint fm = {0};

	TEST_ASSERT_EQUAL_size_t(0, fm.num_buckets);
	TEST_ASSERT_EQUAL_size_t(0, fm.bucket_max);
	TEST_ASSERT_EQUAL_size_t(0, fm.slot_mask);
	TEST_ASSERT_EQUAL_size_t(0, fm.capacity);
	TEST_ASSERT_EQUAL_FLOAT(0.0f, fm.load_factor);
	TEST_ASSERT_EQUAL_size_t(0, fm.load_max);
	TEST_ASSERT_EQUAL_size_t(0, fm.size);
	TEST_ASSERT_NULL(fm.nodes);
	TEST_ASSERT_NULL(fm.info);
}

void test_deinit(void)
{
	struct fmap_intint fm = {0};

	fm.size = 1;
	fm.nodes = tlmalloc(sizeof(struct fmap_intint_node));
	fm.info = tlmalloc(sizeof(enum tl_map_slot_state));
	fmap_intint_deinit(&fm);

	TEST_ASSERT_EQUAL_size_t(0, fm.size);
	TEST_ASSERT_NULL(fm.nodes);
	TEST_ASSERT_NULL(fm.info);
}

void test_init_no_arg(void)
{
	struct fmap_intint fm;
	fmap_intint_init(&fm);

	TEST_ASSERT_EQUAL_size_t(8, fm.num_buckets);
	TEST_ASSERT_EQUAL_size_t(3, fm.bucket_max);
	TEST_ASSERT_EQUAL_size_t(7, fm.slot_mask);
	TEST_ASSERT_EQUAL_size_t(24, fm.capacity);
	TEST_ASSERT_EQUAL_FLOAT(0.7f, fm.load_factor);
	TEST_ASSERT_EQUAL_size_t(16, fm.load_max);
	TEST_ASSERT_EQUAL_size_t(0, fm.size);
	TEST_ASSERT_NOT_NULL(fm.nodes);
	TEST_ASSERT_NOT_NULL(fm.info);

	fmap_intint_deinit(&fm);
}




int main(void)
{
	UNITY_BEGIN();

	RUN_TEST(test_generated_node);
	RUN_TEST(test_generated_type);
	RUN_TEST(test_deinit);
	RUN_TEST(test_init_no_arg);


	return UNITY_END();
}