#include <unity.h>

#include <stdint.h>

#define TL_K int
#define TL_V int
#include "flatmap.h"


/**
 * helpers
 */
int find_key_in_bucket(size_t search, size_t mask, int start_at)
{
	int ret = 0;
	int end = start_at + 10000;
	for (; start_at < end; start_at++) {
		size_t hash = fmap_intint_fnv1a(start_at);
		size_t bucket = hash & mask;
		if (bucket == search) {
			ret = start_at;
			break;
		}
	}

	return ret;
}


/**
 * Testing
 */

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

void test_delete(void)
{
	struct fmap_intint* fm = tlmalloc(sizeof(struct fmap_intint));
	fm->nodes = tlmalloc(sizeof(struct fmap_intint_node));
	fm->info = tlmalloc(sizeof(enum tl_map_slot_state));
	fm->capacity = 1;

	fmap_intint_delete(&fm);
	TEST_ASSERT_NULL(fm);
}

void test_new_no_arg(void)
{
	struct fmap_intint* fm = fmap_intint_new();

	TEST_ASSERT_EQUAL_size_t(8, fm->num_buckets);
	TEST_ASSERT_EQUAL_size_t(3, fm->bucket_max);
	TEST_ASSERT_EQUAL_size_t(7, fm->slot_mask);
	TEST_ASSERT_EQUAL_size_t(24, fm->capacity);
	TEST_ASSERT_EQUAL_FLOAT(0.7f, fm->load_factor);
	TEST_ASSERT_EQUAL_size_t(16, fm->load_max);
	TEST_ASSERT_EQUAL_size_t(0, fm->size);
	TEST_ASSERT_NOT_NULL(fm->nodes);
	TEST_ASSERT_NOT_NULL(fm->info);

	fmap_intint_delete(&fm);
}

void test_grow(void)
{
	struct fmap_intint fm;
	fmap_intint_init(&fm);

	fmap_intint_grow(&fm);

	TEST_ASSERT_EQUAL_size_t(16, fm.num_buckets);
	TEST_ASSERT_EQUAL_size_t(4, fm.bucket_max);
	TEST_ASSERT_EQUAL_size_t(15, fm.slot_mask);
	TEST_ASSERT_EQUAL_size_t(64, fm.capacity);
	TEST_ASSERT_EQUAL_FLOAT(0.7f, fm.load_factor);
	TEST_ASSERT_EQUAL_size_t(44, fm.load_max);
	TEST_ASSERT_EQUAL_size_t(0, fm.size);
	TEST_ASSERT_NOT_NULL(fm.nodes);
	TEST_ASSERT_NOT_NULL(fm.info);

	fmap_intint_deinit(&fm);
}




/**********************************************************************************************************************
 * add Tests
 **********************************************************************************************************************/

void test_add_one(void)
{
	struct fmap_intint fm;
	fmap_intint_init_all(&fm, 8, 0.7f);

	/**
	 * tset
	 * hashes to 0x50d090ef4acbcc21
	 * goes to slot 1
	 * which means 1 * bucket_max = 3
	 */
	int key = 1952805748;
	int value = 10;

	fmap_intint_add(&fm, key, value);

	TEST_ASSERT_EQUAL_size_t(8, fm.num_buckets);
	TEST_ASSERT_EQUAL_size_t(3, fm.bucket_max);
	TEST_ASSERT_EQUAL_size_t(7, fm.slot_mask);
	TEST_ASSERT_EQUAL_size_t(24, fm.capacity);
	TEST_ASSERT_EQUAL_FLOAT(0.7f, fm.load_factor);
	TEST_ASSERT_EQUAL_size_t(16, fm.load_max);
	TEST_ASSERT_EQUAL_size_t(1, fm.size);
	TEST_ASSERT_NOT_NULL(fm.nodes);
	TEST_ASSERT_NOT_NULL(fm.info);

	TEST_ASSERT_EQUAL_INT(1952805748, fm.nodes[3].key);
	TEST_ASSERT_EQUAL_INT(10, fm.nodes[3].value);
	TEST_ASSERT_EQUAL_INT(TL_MAPSS_OCCUPIED, fm.info[3]);

	fmap_intint_deinit(&fm);
}

void test_add_begin(void)
{
	struct fmap_intint fm;
	fmap_intint_init_all(&fm, 8, 0.7f);

	int key = find_key_in_bucket(0, fm.slot_mask, 11111);
	fmap_intint_add(&fm, key, 10);

	TEST_ASSERT_EQUAL_size_t(1, fm.size);
	TEST_ASSERT_EQUAL_INT(key, fm.nodes[0].key);
	TEST_ASSERT_EQUAL_INT(10, fm.nodes[0].value);
	TEST_ASSERT_EQUAL_INT(TL_MAPSS_OCCUPIED, fm.info[0]);

	fmap_intint_deinit(&fm);
}

void test_add_end(void)
{
	struct fmap_intint fm;
	fmap_intint_init_all(&fm, 8, 0.7f);

	int key = find_key_in_bucket(7, fm.slot_mask, 11111);
	fmap_intint_add(&fm, key, 10);

	size_t pos = 7 * fm.bucket_max;
	TEST_ASSERT_EQUAL_size_t(1, fm.size);
	TEST_ASSERT_EQUAL_INT(key, fm.nodes[pos].key);
	TEST_ASSERT_EQUAL_INT(10, fm.nodes[pos].value);
	TEST_ASSERT_EQUAL_INT(TL_MAPSS_OCCUPIED, fm.info[pos]);

	fmap_intint_deinit(&fm);
}

void test_add_two_diff_buckets(void)
{
	struct fmap_intint fm;
	fmap_intint_init_all(&fm, 8, 0.7f);

	int key = find_key_in_bucket(2, fm.slot_mask, 100000);
	fmap_intint_add(&fm, key, 10);

	int key2 = find_key_in_bucket(4, fm.slot_mask, 100000);
	fmap_intint_add(&fm, key2, 20);

	TEST_ASSERT_EQUAL_size_t(2, fm.size);
	TEST_ASSERT_EQUAL_INT(key, fm.nodes[2 * fm.bucket_max].key);
	TEST_ASSERT_EQUAL_INT(10, fm.nodes[2 * fm.bucket_max].value);
	TEST_ASSERT_EQUAL_INT(TL_MAPSS_OCCUPIED, fm.info[2 * fm.bucket_max]);

	TEST_ASSERT_EQUAL_INT(key2, fm.nodes[4 * fm.bucket_max].key);
	TEST_ASSERT_EQUAL_INT(20, fm.nodes[4 * fm.bucket_max].value);
	TEST_ASSERT_EQUAL_INT(TL_MAPSS_OCCUPIED, fm.info[4 * fm.bucket_max]);

	fmap_intint_deinit(&fm);
}

void test_add_two_same_bucket(void)
{
	struct fmap_intint fm;
	fmap_intint_init_all(&fm, 8, 0.7f);

	int key = find_key_in_bucket(2, fm.slot_mask, 100000);
	fmap_intint_add(&fm, key, 10);

	int key2 = find_key_in_bucket(2, fm.slot_mask, 1000000);
	fmap_intint_add(&fm, key2, 20);

	TEST_ASSERT_EQUAL_size_t(2, fm.size);
	size_t pos = 2 * fm.bucket_max;
	TEST_ASSERT_EQUAL_INT(key, fm.nodes[pos].key);
	TEST_ASSERT_EQUAL_INT(10, fm.nodes[pos].value);
	TEST_ASSERT_EQUAL_INT(TL_MAPSS_OCCUPIED, fm.info[pos]);

	size_t pos2 = pos + 1;
	TEST_ASSERT_EQUAL_INT(key2, fm.nodes[pos2].key);
	TEST_ASSERT_EQUAL_INT(20, fm.nodes[pos2].value);
	TEST_ASSERT_EQUAL_INT(TL_MAPSS_COLLIDED, fm.info[pos2]);

	fmap_intint_deinit(&fm);
}

void test_add_two_same_bucket_begin(void)
{
	struct fmap_intint fm;
	fmap_intint_init_all(&fm, 8, 0.7f);

	int key = find_key_in_bucket(0, fm.slot_mask, 100000);
	fmap_intint_add(&fm, key, 10);

	int key2 = find_key_in_bucket(0, fm.slot_mask, 1000000);
	fmap_intint_add(&fm, key2, 20);

	TEST_ASSERT_EQUAL_size_t(2, fm.size);
	size_t pos = 0;
	TEST_ASSERT_EQUAL_INT(key, fm.nodes[pos].key);
	TEST_ASSERT_EQUAL_INT(10, fm.nodes[pos].value);
	TEST_ASSERT_EQUAL_INT(TL_MAPSS_OCCUPIED, fm.info[pos]);

	size_t pos2 = pos + 1;
	TEST_ASSERT_EQUAL_INT(key2, fm.nodes[pos2].key);
	TEST_ASSERT_EQUAL_INT(20, fm.nodes[pos2].value);
	TEST_ASSERT_EQUAL_INT(TL_MAPSS_COLLIDED, fm.info[pos2]);

	fmap_intint_deinit(&fm);
}

void test_add_two_same_key(void)
{
	struct fmap_intint fm;
	fmap_intint_init_all(&fm, 8, 0.7f);

	int key = find_key_in_bucket(2, fm.slot_mask, 100000);
	fmap_intint_add(&fm, key, 10);

	TEST_ASSERT_EQUAL_INT(TL_EAE, fmap_intint_add(&fm, key, 20));

	TEST_ASSERT_EQUAL_size_t(1, fm.size);
	size_t pos = 2 * fm.bucket_max;
	TEST_ASSERT_EQUAL_INT(key, fm.nodes[pos].key);
	TEST_ASSERT_EQUAL_INT(10, fm.nodes[pos].value);
	TEST_ASSERT_EQUAL_INT(TL_MAPSS_OCCUPIED, fm.info[pos]);

	size_t pos2 = pos + 1;
	TEST_ASSERT_EQUAL_INT(0, fm.nodes[pos2].key);
	TEST_ASSERT_EQUAL_INT(0, fm.nodes[pos2].value);
	TEST_ASSERT_EQUAL_INT(TL_MAPSS_EMPTY, fm.info[pos2]);

	fmap_intint_deinit(&fm);
}

void test_add_fill_bucket(void)
{
	struct fmap_intint fm;
	fmap_intint_init_all(&fm, 8, 0.7f);


	for (size_t i = 1; i <= fm.bucket_max; i++) {
		int key = find_key_in_bucket(2, fm.slot_mask, i * 100000);
		enum tl_status val = fmap_intint_add(&fm, key, i * 10);

		TEST_ASSERT_EQUAL_INT(TLOK, val);
	}

	//we expect that a grow will not have occurred!
	TEST_ASSERT_EQUAL_size_t(fm.bucket_max, fm.size);
	TEST_ASSERT_EQUAL_size_t(8, fm.num_buckets);
	TEST_ASSERT_EQUAL_size_t(fm.num_buckets * fm.bucket_max, fm.capacity);


	fmap_intint_deinit(&fm);
}

void test_add_fill_bucket_then_diff_bucket(void)
{
	struct fmap_intint fm;
	fmap_intint_init_all(&fm, 8, 0.7f);


	for (size_t i = 1; i <= fm.bucket_max; i++) {
		int key = find_key_in_bucket(2, fm.slot_mask, i * 100000);
		enum tl_status val = fmap_intint_add(&fm, key, i * 10);

		TEST_ASSERT_EQUAL_INT(TLOK, val);
	}

	int key = find_key_in_bucket(3, fm.slot_mask, 100000);
	enum tl_status val = fmap_intint_add(&fm, key, 60);

	TEST_ASSERT_EQUAL_INT(TLOK, val);

	//we expect that a grow will not have occurred!
	TEST_ASSERT_EQUAL_size_t(fm.bucket_max + 1, fm.size);
	TEST_ASSERT_EQUAL_size_t(8, fm.num_buckets);
	TEST_ASSERT_EQUAL_size_t(fm.num_buckets * fm.bucket_max, fm.capacity);


	fmap_intint_deinit(&fm);
}

void test_grow_by_full_bucket(void)
{
	struct fmap_intint fm;
	fmap_intint_init_all(&fm, 8, 0.7f);
	size_t old_bucket_max = fm.bucket_max;

	int keys[30];

	for (size_t i = 1; i <= fm.bucket_max; i++) {
		keys[i - 1] = find_key_in_bucket(2, fm.slot_mask, i * 100000);
		enum tl_status val = fmap_intint_add(&fm, keys[i - 1], i * 10);

		TEST_ASSERT_EQUAL_INT(TLOK, val);
	}

	keys[old_bucket_max] = find_key_in_bucket(2, fm.slot_mask, 11111);
	enum tl_status val = fmap_intint_add(&fm, keys[old_bucket_max], 60);

	for (size_t i = 0; i < old_bucket_max + 1; i++) {
		size_t hash = fmap_intint_fnv1a(keys[i]);
		size_t bucket = hash & fm.slot_mask;
		size_t slot = bucket * fm.bucket_max;
		size_t slot_i = 0;
		enum tl_status val = fmap_intint_probe_key(fm.nodes, fm.info, slot, fm.bucket_max, keys[i], &slot_i);
		TEST_ASSERT_EQUAL_INT(TLOK, val);
	}

	TEST_ASSERT_EQUAL_INT(TLOK, val);

	//we expect that a grow will have occurred!
	TEST_ASSERT_EQUAL_size_t(16, fm.num_buckets);
	TEST_ASSERT_EQUAL_size_t(old_bucket_max + 1, fm.size);
	TEST_ASSERT_EQUAL_size_t(fm.num_buckets * fm.bucket_max, fm.capacity);


	fmap_intint_deinit(&fm);
}

void test_grow_into_beginning(void)
{
	struct fmap_intint fm;
	fmap_intint_init_all(&fm, 8, 0.7f);
	size_t old_bucket_max = fm.bucket_max;

	int keys[30];

	/**
	 * keep in mind that if the subsequent bucket will be 0, then the current bucket is also
	 * guaranteed to be 0
	 */
	for (size_t i = 1; i <= fm.bucket_max; i++) {
		keys[i - 1] = find_key_in_bucket(0, 15, i * 100000);
		enum tl_status val = fmap_intint_add(&fm, keys[i - 1], i * 10);

		TEST_ASSERT_EQUAL_INT(TLOK, val);
	}

	keys[old_bucket_max] = find_key_in_bucket(0, 15, 11111);
	enum tl_status val = fmap_intint_add(&fm, keys[old_bucket_max], 60);

	for (size_t i = 0; i < old_bucket_max + 1; i++) {
		size_t hash = fmap_intint_fnv1a(keys[i]);
		size_t bucket = hash & fm.slot_mask;
		size_t slot = bucket * fm.bucket_max;
		size_t slot_i = 0;
		enum tl_status val = fmap_intint_probe_key(fm.nodes, fm.info, slot, fm.bucket_max, keys[i], &slot_i);
		TEST_ASSERT_EQUAL_INT(TLOK, val);
	}

	TEST_ASSERT_EQUAL_INT(TLOK, val);

	//we expect that a grow will have occurred!
	TEST_ASSERT_EQUAL_size_t(16, fm.num_buckets);
	TEST_ASSERT_EQUAL_size_t(old_bucket_max + 1, fm.size);
	TEST_ASSERT_EQUAL_size_t(fm.num_buckets * fm.bucket_max, fm.capacity);


	fmap_intint_deinit(&fm);
}






int main(void)
{
	UNITY_BEGIN();

	RUN_TEST(test_generated_node);
	RUN_TEST(test_generated_type);
	RUN_TEST(test_deinit);
	RUN_TEST(test_init_no_arg);
	RUN_TEST(test_delete);
	RUN_TEST(test_new_no_arg);
	RUN_TEST(test_grow);

	RUN_TEST(test_add_one);
	RUN_TEST(test_add_begin);
	RUN_TEST(test_add_end);
	RUN_TEST(test_add_two_diff_buckets);
	RUN_TEST(test_add_two_same_bucket);
	RUN_TEST(test_add_two_same_bucket_begin);
	RUN_TEST(test_add_two_same_key);
	RUN_TEST(test_add_fill_bucket);
	RUN_TEST(test_add_fill_bucket_then_diff_bucket);
	RUN_TEST(test_grow_by_full_bucket);
	RUN_TEST(test_grow_into_beginning);



	return UNITY_END();
}