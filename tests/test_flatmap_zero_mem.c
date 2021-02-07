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

int* generate_list(size_t count, int unique)
{
	int* list = tlmalloc(count * sizeof(int));

	size_t i = 0;
	int found;
	while (i < count) {
		int val = (int)random();
		found = 0;

		if (unique) {
			for (size_t j = 0; j < i; j++) {
				if (list[j] == val) {
					found = 1;
					break;
				}
			}
			if (found) {
				continue;
			}
		}
		list[i] = val;
		i++;
	}

	return list;
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
	TEST_ASSERT_EQUAL_size_t(70u, fm.load_factor);
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
	TEST_ASSERT_EQUAL_size_t(70u, fm->load_factor);
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
	TEST_ASSERT_EQUAL_size_t(70u, fm.load_factor);
	TEST_ASSERT_EQUAL_size_t(44, fm.load_max);
	TEST_ASSERT_EQUAL_size_t(0, fm.size);
	TEST_ASSERT_NOT_NULL(fm.nodes);
	TEST_ASSERT_NOT_NULL(fm.info);

	fmap_intint_deinit(&fm);
}

void test_grow_with_one_ele(void)
{
	struct fmap_intint fm;
	fmap_intint_init_all(&fm, 8, 70);

	int key = find_key_in_bucket(14, 15, 54321);
	size_t hash = fmap_intint_fnv1a(key);
	size_t bucket = hash & fm.slot_mask;
	size_t pos = bucket * fm.bucket_max;

	fm.nodes[pos].key = key;
	fm.nodes[pos].value = 87598;
	fm.info[pos] =TL_MAPSS_OCCUPIED;
	fm.size++;

	fmap_intint_grow(&fm);
	pos = 14 * fm.bucket_max;

	TEST_ASSERT_EQUAL_INT(key, fm.nodes[pos].key);
	TEST_ASSERT_EQUAL_INT(TL_MAPSS_OCCUPIED, fm.info[pos]);
	TEST_ASSERT_EQUAL_size_t(1, fm.size);

	fmap_intint_deinit(&fm);
}





/**********************************************************************************************************************
 * add Tests
 **********************************************************************************************************************/

void test_add_one(void)
{
	struct fmap_intint fm;
	fmap_intint_init_all(&fm, 8, 70u);

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
	TEST_ASSERT_EQUAL_FLOAT(70u, fm.load_factor);
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
	fmap_intint_init_all(&fm, 8, 70u);

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
	fmap_intint_init_all(&fm, 8, 70u);

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
	fmap_intint_init_all(&fm, 8, 70u);

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
	fmap_intint_init_all(&fm, 8, 70u);

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
	fmap_intint_init_all(&fm, 8, 70u);

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
	fmap_intint_init_all(&fm, 8, 70u);

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
	fmap_intint_init_all(&fm, 8, 70u);


	for (size_t i = 1; i <= fm.bucket_max; i++) {
		int key = find_key_in_bucket(2, fm.slot_mask, i * 100000);
		enum tl_status val = fmap_intint_add(&fm, key, i * 10);

		TEST_ASSERT_EQUAL_INT(TLOK, val);
	}

	/* we expect that a grow will not have occurred! */
	TEST_ASSERT_EQUAL_size_t(fm.bucket_max, fm.size);
	TEST_ASSERT_EQUAL_size_t(8, fm.num_buckets);
	TEST_ASSERT_EQUAL_size_t(fm.num_buckets * fm.bucket_max, fm.capacity);


	fmap_intint_deinit(&fm);
}

void test_add_fill_bucket_then_diff_bucket(void)
{
	struct fmap_intint fm;
	fmap_intint_init_all(&fm, 8, 70u);


	for (size_t i = 1; i <= fm.bucket_max; i++) {
		int key = find_key_in_bucket(2, fm.slot_mask, i * 100000);
		enum tl_status val = fmap_intint_add(&fm, key, i * 10);

		TEST_ASSERT_EQUAL_INT(TLOK, val);
	}

	int key = find_key_in_bucket(3, fm.slot_mask, 100000);
	enum tl_status val = fmap_intint_add(&fm, key, 60);

	TEST_ASSERT_EQUAL_INT(TLOK, val);

	/* we expect that a grow will not have occurred! */
	TEST_ASSERT_EQUAL_size_t(fm.bucket_max + 1, fm.size);
	TEST_ASSERT_EQUAL_size_t(8, fm.num_buckets);
	TEST_ASSERT_EQUAL_size_t(fm.num_buckets * fm.bucket_max, fm.capacity);


	fmap_intint_deinit(&fm);
}

void test_add_grow_by_full_bucket(void)
{
	struct fmap_intint fm;
	fmap_intint_init_all(&fm, 8, 70u);
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

	/* we expect that a grow will have occurred! */
	TEST_ASSERT_EQUAL_size_t(16, fm.num_buckets);
	TEST_ASSERT_EQUAL_size_t(old_bucket_max + 1, fm.size);
	TEST_ASSERT_EQUAL_size_t(fm.num_buckets * fm.bucket_max, fm.capacity);


	fmap_intint_deinit(&fm);
}

void test_add_grow_into_beginning(void)
{
	struct fmap_intint fm;
	fmap_intint_init_all(&fm, 8, 70u);
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

	TEST_ASSERT_EQUAL_size_t(8, fm.num_buckets);
	TEST_ASSERT_EQUAL_size_t(old_bucket_max, fm.size);

	keys[old_bucket_max] = find_key_in_bucket(0, 15, 11111);
	enum tl_status val = fmap_intint_add(&fm, keys[old_bucket_max], 60);

	for (size_t i = 0; i < old_bucket_max + 1; i++) {
		size_t hash = fmap_intint_fnv1a(keys[i]);
		size_t bucket = hash & fm.slot_mask;
		size_t slot = bucket * fm.bucket_max;
		size_t slot_i = 0;
		enum tl_status ret = fmap_intint_probe_key(fm.nodes, fm.info, slot, fm.bucket_max, keys[i], &slot_i);
		TEST_ASSERT_EQUAL_INT(TLOK, ret);
	}

	TEST_ASSERT_EQUAL_INT(TLOK, val);

	/* we expect that a grow will have occurred! */
	TEST_ASSERT_EQUAL_size_t(16, fm.num_buckets);
	TEST_ASSERT_EQUAL_size_t(old_bucket_max + 1, fm.size);
	TEST_ASSERT_EQUAL_size_t(fm.num_buckets * fm.bucket_max, fm.capacity);

	fmap_intint_deinit(&fm);
}

void test_add_grow_into_end(void)
{
	struct fmap_intint fm;
	fmap_intint_init_all(&fm, 8, 70u);
	size_t old_bucket_max = fm.bucket_max;

	int keys[30];

	/**
	 * keep in mind that if the subsequent bucket will be 15, then the earlier must also be the end
	 * bucket
	 */
	for (size_t i = 1; i <= fm.bucket_max; i++) {
		keys[i - 1] = find_key_in_bucket(15, 15, i * 100000);
		enum tl_status val = fmap_intint_add(&fm, keys[i - 1], i * 10);

		TEST_ASSERT_EQUAL_INT(TLOK, val);
	}

	TEST_ASSERT_EQUAL_size_t(8, fm.num_buckets);
	TEST_ASSERT_EQUAL_size_t(old_bucket_max, fm.size);


	keys[old_bucket_max] = find_key_in_bucket(15, 15, 11111);
	enum tl_status val = fmap_intint_add(&fm, keys[old_bucket_max], 60);

	for (size_t i = 0; i < old_bucket_max + 1; i++) {
		size_t hash = fmap_intint_fnv1a(keys[i]);
		size_t bucket = hash & fm.slot_mask;
		size_t slot = bucket * fm.bucket_max;
		size_t slot_i = 0;
		enum tl_status ret = fmap_intint_probe_key(fm.nodes, fm.info, slot, fm.bucket_max, keys[i], &slot_i);
		TEST_ASSERT_EQUAL_INT(TLOK, ret);
	}

	TEST_ASSERT_EQUAL_INT(TLOK, val);

	/* we expect that a grow will have occurred! */
	TEST_ASSERT_EQUAL_size_t(16, fm.num_buckets);
	TEST_ASSERT_EQUAL_size_t(old_bucket_max + 1, fm.size);
	TEST_ASSERT_EQUAL_size_t(fm.num_buckets * fm.bucket_max, fm.capacity);

	fmap_intint_deinit(&fm);
}

void test_add_grow_size_bound(void)
{
	struct fmap_intint fm;
	fmap_intint_init_all(&fm, 8, 70u);

	size_t bound = (fm.num_buckets * fm.bucket_max * fm.load_factor) / 100;
	int bucket = 0;

	/**
	 * Distribute in to a guaranteed bucket up till the grow boundary
	 */
	for (int i = 0; i < bound; i++, bucket++) {
		if (bucket >= fm.num_buckets)
			bucket = 0;

		int key = find_key_in_bucket(bucket, fm.slot_mask, 12345 * i + 1);
		fmap_intint_add(&fm, key, 10 * (i + 1));
	}

	TEST_ASSERT_EQUAL_size_t(8, fm.num_buckets);
	TEST_ASSERT_EQUAL_size_t(bound, fm.size);

	fmap_intint_deinit(&fm);
}

void test_add_over_grow_bound(void)
{
	struct fmap_intint fm;
	fmap_intint_init_all(&fm, 8, 70u);

	size_t bound = (fm.num_buckets * fm.bucket_max * fm.load_factor) / 100;
	int bucket = 0;

	/**
	 * Distribute in to a guaranteed bucket up till the grow boundary
	 */
	for (int i = 0; i < bound; i++, bucket++) {
		if (bucket >= fm.num_buckets)
			bucket = 0;

		int key = find_key_in_bucket(bucket, fm.slot_mask, 12345 * i + 1);
		fmap_intint_add(&fm, key, 10 * (i + 1));
	}

	TEST_ASSERT_EQUAL_size_t(8, fm.num_buckets);
	TEST_ASSERT_EQUAL_size_t(bound, fm.size);

	int key = find_key_in_bucket(bucket, fm.slot_mask, 12345 * (int)bound + 1);
	fmap_intint_add(&fm, key, 10 * ((int)bound + 1));

	TEST_ASSERT_EQUAL_size_t(16, fm.num_buckets);
	TEST_ASSERT_EQUAL_size_t(bound + 1u, fm.size);
	TEST_ASSERT_EQUAL_size_t(44, fm.load_max);

	fmap_intint_deinit(&fm);
}





/**********************************************************************************************************************
 * get Tests
 **********************************************************************************************************************/

void test_get_one(void)
{
	struct fmap_intint fm;
	fmap_intint_init(&fm);

	fmap_intint_add(&fm, 9876543, 101);

	int value = fmap_intint_get(&fm, 9876543);

	TEST_ASSERT_EQUAL_INT(101, value);

	fmap_intint_deinit(&fm);
}

void test_get_begin(void)
{
	struct fmap_intint fm;
	fmap_intint_init(&fm);

	int key = find_key_in_bucket(0, fm.slot_mask, 758929);
	fmap_intint_add(&fm, key, 101);

	int value = fmap_intint_get(&fm, key);

	TEST_ASSERT_EQUAL_INT(101, value);

	fmap_intint_deinit(&fm);
}

void test_get_last_bucket(void)
{
	struct fmap_intint fm;
	fmap_intint_init(&fm);

	int key = find_key_in_bucket(fm.bucket_max - 1, fm.slot_mask, 758929);
	fmap_intint_add(&fm, key, 101);

	int value = fmap_intint_get(&fm, key);

	TEST_ASSERT_EQUAL_INT(101, value);

	fmap_intint_deinit(&fm);
}

void test_get_end_of_begin(void)
{
	struct fmap_intint fm;
	fmap_intint_init(&fm);

	int keys[fm.bucket_max];
	for (int i = 0; i < fm.bucket_max; i++) {
		keys[i] = find_key_in_bucket(0, fm.slot_mask, 4757 * i);
		fmap_intint_add(&fm, keys[i], 101 + i);
	}

	for (int i = 0; i < fm.bucket_max; i++) {
		int value = fmap_intint_get(&fm, keys[i]);

		TEST_ASSERT_EQUAL_INT(101 + i, value);
	}


	fmap_intint_deinit(&fm);
}

void test_get_end_of_end(void)
{
	struct fmap_intint fm;
	fmap_intint_init_all(&fm, 16, 70u);

	int keys[fm.bucket_max];
	for (int i = 0; i < fm.bucket_max; i++) {
		keys[i] = find_key_in_bucket(15, fm.slot_mask, 4757 * i);
		fmap_intint_add(&fm, keys[i], 101 + i);
	}

	for (int i = 0; i < fm.bucket_max; i++) {
		int value = fmap_intint_get(&fm, keys[i]);

		TEST_ASSERT_EQUAL_INT(101 + i, value);
	}


	fmap_intint_deinit(&fm);
}

void test_get_after_grow(void)
{
	struct fmap_intint fm;
	fmap_intint_init_all(&fm, 16, 70u);

	size_t capacity = fm.load_max + 1;
	int* keys = generate_list(capacity, 1);
	for (int i = 0; i < capacity; i++) {
		fmap_intint_add(&fm, keys[i], 101 + i);
	}

	TEST_ASSERT_EQUAL_size_t(capacity, fm.size);
	TEST_ASSERT_NOT_EQUAL_size_t(capacity, fm.capacity);	/* confirms a grow happened. */

	for (int i = 0; i < fm.bucket_max; i++) {
		int value = fmap_intint_get(&fm, keys[i]);

		TEST_ASSERT_EQUAL_INT(101 + i, value);
	}

	tlfree(keys);
	fmap_intint_deinit(&fm);
}

void test_get_after_many_grow(void)
{
	struct fmap_intint fm;
	fmap_intint_init_all(&fm, 8, 70u);

	size_t capacity = 2000;
	int* keys = generate_list(capacity, 1);

	for (int i = 0; i < capacity; i++) {
		fmap_intint_add(&fm, keys[i], 101 + i);
	}

	TEST_ASSERT_EQUAL_size_t(capacity, fm.size);

	for (int i = 0; i < fm.bucket_max; i++) {
		int value = fmap_intint_get(&fm, keys[i]);

		TEST_ASSERT_EQUAL_INT(101 + i, value);
	}


	tlfree(keys);
	fmap_intint_deinit(&fm);
}





/**********************************************************************************************************************
 * try_get Tests
 **********************************************************************************************************************/

void test_try_get_one(void)
{
	struct fmap_intint fm;
	fmap_intint_init(&fm);

	fmap_intint_add(&fm, 9876543, 101);
	int x;

	TEST_ASSERT_EQUAL_INT(TLOK, fmap_intint_try_get(&fm, 9876543, &x));

	TEST_ASSERT_EQUAL_INT(101, x);

	fmap_intint_deinit(&fm);
}

void test_try_get_begin(void)
{
	struct fmap_intint fm;
	fmap_intint_init(&fm);

	int key = find_key_in_bucket(0, fm.slot_mask, 758929);
	fmap_intint_add(&fm, key, 101);
	int x;

	TEST_ASSERT_EQUAL_INT(TLOK, fmap_intint_try_get(&fm, key, &x));

	TEST_ASSERT_EQUAL_INT(101, x);

	fmap_intint_deinit(&fm);
}

void test_try_get_missing(void)
{
	struct fmap_intint fm;
	fmap_intint_init(&fm);

	int key = find_key_in_bucket(0, fm.slot_mask, 758929);
	fmap_intint_add(&fm, key, 101);
	int x = 0;

	TEST_ASSERT_EQUAL_INT(TL_ENF, fmap_intint_try_get(&fm, 475678, &x));

	TEST_ASSERT_EQUAL_INT(0, x);

	fmap_intint_deinit(&fm);
}

void test_try_get_last_bucket(void)
{
	struct fmap_intint fm;
	fmap_intint_init(&fm);

	int key = find_key_in_bucket(fm.bucket_max - 1, fm.slot_mask, 758929);
	fmap_intint_add(&fm, key, 101);
	int x;

	TEST_ASSERT_EQUAL_INT(TLOK, fmap_intint_try_get(&fm, key, &x));

	TEST_ASSERT_EQUAL_INT(101, x);

	fmap_intint_deinit(&fm);
}

void test_try_get_end_of_begin(void)
{
	struct fmap_intint fm;
	fmap_intint_init(&fm);

	int keys[fm.bucket_max];
	for (int i = 0; i < fm.bucket_max; i++) {
		keys[i] = find_key_in_bucket(0, fm.slot_mask, 4757 * i);
		fmap_intint_add(&fm, keys[i], 101 + i);
	}

	for (int i = 0; i < fm.bucket_max; i++) {
		int x;
		TEST_ASSERT_EQUAL_INT(TLOK, fmap_intint_try_get(&fm, keys[i], &x));

		TEST_ASSERT_EQUAL_INT(101 + i, x);
	}


	fmap_intint_deinit(&fm);
}

void test_try_get_end_of_end(void)
{
	struct fmap_intint fm;
	fmap_intint_init_all(&fm, 16, 70u);

	int keys[fm.bucket_max];
	for (int i = 0; i < fm.bucket_max; i++) {
		keys[i] = find_key_in_bucket(15, fm.slot_mask, 4757 * i);
		fmap_intint_add(&fm, keys[i], 101 + i);
	}

	for (int i = 0; i < fm.bucket_max; i++) {
		int x;
		TEST_ASSERT_EQUAL_INT(TLOK, fmap_intint_try_get(&fm, keys[i], &x));
		TEST_ASSERT_EQUAL_INT(101 + i, x);
	}


	fmap_intint_deinit(&fm);
}

void test_try_get_after_grow(void)
{
	struct fmap_intint fm;
	fmap_intint_init_all(&fm, 16, 70u);

	size_t capacity = fm.load_max + 1;
	int* keys = generate_list(capacity, 1);
	for (int i = 0; i < capacity; i++) {
		fmap_intint_add(&fm, keys[i], 101 + i);
	}

	TEST_ASSERT_EQUAL_size_t(capacity, fm.size);
	TEST_ASSERT_NOT_EQUAL_size_t(capacity, fm.capacity);	/* confirms a grow happened. */

	for (int i = 0; i < fm.bucket_max; i++) {
		int x;
		TEST_ASSERT_EQUAL_INT(TLOK, fmap_intint_try_get(&fm, keys[i], &x));

		TEST_ASSERT_EQUAL_INT(101 + i, x);
	}

	tlfree(keys);
	fmap_intint_deinit(&fm);
}

void test_try_get_after_many_grow(void)
{
	struct fmap_intint fm;
	fmap_intint_init_all(&fm, 8, 70u);

	size_t capacity = 2000;
	int* keys = generate_list(capacity, 1);

	for (int i = 0; i < capacity; i++) {
		fmap_intint_add(&fm, keys[i], 101 + i);
	}

	TEST_ASSERT_EQUAL_size_t(capacity, fm.size);

	for (int i = 0; i < fm.bucket_max; i++) {
		int x;
		TEST_ASSERT_EQUAL_INT(TLOK, fmap_intint_try_get(&fm, keys[i], &x));

		TEST_ASSERT_EQUAL_INT(101 + i, x);
	}


	tlfree(keys);
	fmap_intint_deinit(&fm);
}






/**********************************************************************************************************************
 * insert Tests
 **********************************************************************************************************************/

void test_insert_one(void)
{
	struct fmap_intint fm;
	fmap_intint_init_all(&fm, 8, 70u);

	/**
	 * tset
	 * hashes to 0x50d090ef4acbcc21
	 * goes to slot 1
	 * which means 1 * bucket_max = 3
	 */
	int key = 1952805748;
	int value = 10;

	TEST_ASSERT_EQUAL_INT(TLOK, fmap_intint_insert(&fm, key, value));

	TEST_ASSERT_EQUAL_size_t(8, fm.num_buckets);
	TEST_ASSERT_EQUAL_size_t(3, fm.bucket_max);
	TEST_ASSERT_EQUAL_size_t(7, fm.slot_mask);
	TEST_ASSERT_EQUAL_size_t(24, fm.capacity);
	TEST_ASSERT_EQUAL_FLOAT(70u, fm.load_factor);
	TEST_ASSERT_EQUAL_size_t(16, fm.load_max);
	TEST_ASSERT_EQUAL_size_t(1, fm.size);
	TEST_ASSERT_NOT_NULL(fm.nodes);
	TEST_ASSERT_NOT_NULL(fm.info);

	TEST_ASSERT_EQUAL_INT(1952805748, fm.nodes[3].key);
	TEST_ASSERT_EQUAL_INT(10, fm.nodes[3].value);
	TEST_ASSERT_EQUAL_INT(TL_MAPSS_OCCUPIED, fm.info[3]);

	fmap_intint_deinit(&fm);
}

void test_insert_begin(void)
{
	struct fmap_intint fm;
	fmap_intint_init_all(&fm, 8, 70u);

	int key = find_key_in_bucket(0, fm.slot_mask, 11111);
	fmap_intint_insert(&fm, key, 10);

	TEST_ASSERT_EQUAL_size_t(1, fm.size);
	TEST_ASSERT_EQUAL_INT(key, fm.nodes[0].key);
	TEST_ASSERT_EQUAL_INT(10, fm.nodes[0].value);
	TEST_ASSERT_EQUAL_INT(TL_MAPSS_OCCUPIED, fm.info[0]);

	fmap_intint_deinit(&fm);
}

void test_insert_end(void)
{
	struct fmap_intint fm;
	fmap_intint_init_all(&fm, 8, 70u);

	int key = find_key_in_bucket(7, fm.slot_mask, 11111);
	fmap_intint_insert(&fm, key, 10);

	size_t pos = 7 * fm.bucket_max;
	TEST_ASSERT_EQUAL_size_t(1, fm.size);
	TEST_ASSERT_EQUAL_INT(key, fm.nodes[pos].key);
	TEST_ASSERT_EQUAL_INT(10, fm.nodes[pos].value);
	TEST_ASSERT_EQUAL_INT(TL_MAPSS_OCCUPIED, fm.info[pos]);

	fmap_intint_deinit(&fm);
}

void test_insert_two_diff_buckets(void)
{
	struct fmap_intint fm;
	fmap_intint_init_all(&fm, 8, 70u);

	int key = find_key_in_bucket(2, fm.slot_mask, 100000);
	fmap_intint_insert(&fm, key, 10);

	int key2 = find_key_in_bucket(4, fm.slot_mask, 100000);
	fmap_intint_insert(&fm, key2, 20);

	TEST_ASSERT_EQUAL_size_t(2, fm.size);
	TEST_ASSERT_EQUAL_INT(key, fm.nodes[2 * fm.bucket_max].key);
	TEST_ASSERT_EQUAL_INT(10, fm.nodes[2 * fm.bucket_max].value);
	TEST_ASSERT_EQUAL_INT(TL_MAPSS_OCCUPIED, fm.info[2 * fm.bucket_max]);

	TEST_ASSERT_EQUAL_INT(key2, fm.nodes[4 * fm.bucket_max].key);
	TEST_ASSERT_EQUAL_INT(20, fm.nodes[4 * fm.bucket_max].value);
	TEST_ASSERT_EQUAL_INT(TL_MAPSS_OCCUPIED, fm.info[4 * fm.bucket_max]);

	fmap_intint_deinit(&fm);
}

void test_insert_two_same_bucket(void)
{
	struct fmap_intint fm;
	fmap_intint_init_all(&fm, 8, 70u);

	int key = find_key_in_bucket(2, fm.slot_mask, 100000);
	fmap_intint_insert(&fm, key, 10);

	int key2 = find_key_in_bucket(2, fm.slot_mask, 1000000);
	fmap_intint_insert(&fm, key2, 20);

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

void test_insert_two_same_bucket_begin(void)
{
	struct fmap_intint fm;
	fmap_intint_init_all(&fm, 8, 70u);

	int key = find_key_in_bucket(0, fm.slot_mask, 100000);
	fmap_intint_insert(&fm, key, 10);

	int key2 = find_key_in_bucket(0, fm.slot_mask, 1000000);
	fmap_intint_insert(&fm, key2, 20);

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

void test_insert_two_same_key(void)
{
	struct fmap_intint fm;
	fmap_intint_init_all(&fm, 8, 70u);

	int key = find_key_in_bucket(2, fm.slot_mask, 100000);
	fmap_intint_insert(&fm, key, 10);

	TEST_ASSERT_EQUAL_INT(TLOK, fmap_intint_insert(&fm, key, 20));

	TEST_ASSERT_EQUAL_size_t(1, fm.size);
	size_t pos = 2 * fm.bucket_max;
	TEST_ASSERT_EQUAL_INT(key, fm.nodes[pos].key);
	TEST_ASSERT_EQUAL_INT(20, fm.nodes[pos].value);
	TEST_ASSERT_EQUAL_INT(TL_MAPSS_OCCUPIED, fm.info[pos]);

	size_t pos2 = pos + 1;
	TEST_ASSERT_EQUAL_INT(0, fm.nodes[pos2].key);
	TEST_ASSERT_EQUAL_INT(0, fm.nodes[pos2].value);
	TEST_ASSERT_EQUAL_INT(TL_MAPSS_EMPTY, fm.info[pos2]);

	fmap_intint_deinit(&fm);
}

void test_insert_fill_bucket(void)
{
	struct fmap_intint fm;
	fmap_intint_init_all(&fm, 8, 70u);

	for (size_t i = 1; i <= fm.bucket_max; i++) {
		int key = find_key_in_bucket(2, fm.slot_mask, i * 100000);
		enum tl_status val = fmap_intint_insert(&fm, key, i * 10);

		TEST_ASSERT_EQUAL_INT(TLOK, val);
	}

	/* we expect that a grow will not have occurred! */
	TEST_ASSERT_EQUAL_size_t(fm.bucket_max, fm.size);
	TEST_ASSERT_EQUAL_size_t(8, fm.num_buckets);
	TEST_ASSERT_EQUAL_size_t(fm.num_buckets * fm.bucket_max, fm.capacity);


	fmap_intint_deinit(&fm);
}

void test_insert_fill_bucket_then_diff_bucket(void)
{
	struct fmap_intint fm;
	fmap_intint_init_all(&fm, 8, 70u);


	for (size_t i = 1; i <= fm.bucket_max; i++) {
		int key = find_key_in_bucket(2, fm.slot_mask, i * 100000);
		enum tl_status val = fmap_intint_insert(&fm, key, i * 10);

		TEST_ASSERT_EQUAL_INT(TLOK, val);
	}

	int key = find_key_in_bucket(3, fm.slot_mask, 100000);
	enum tl_status val = fmap_intint_insert(&fm, key, 60);

	TEST_ASSERT_EQUAL_INT(TLOK, val);

	/* we expect that a grow will not have occurred! */
	TEST_ASSERT_EQUAL_size_t(fm.bucket_max + 1, fm.size);
	TEST_ASSERT_EQUAL_size_t(8, fm.num_buckets);
	TEST_ASSERT_EQUAL_size_t(fm.num_buckets * fm.bucket_max, fm.capacity);


	fmap_intint_deinit(&fm);
}

void test_insert_grow_by_full_bucket(void)
{
	struct fmap_intint fm;
	fmap_intint_init_all(&fm, 8, 70u);
	size_t old_bucket_max = fm.bucket_max;

	int keys[30];

	for (size_t i = 1; i <= fm.bucket_max; i++) {
		keys[i - 1] = find_key_in_bucket(2, fm.slot_mask, i * 100000);
		enum tl_status val = fmap_intint_insert(&fm, keys[i - 1], i * 10);

		TEST_ASSERT_EQUAL_INT(TLOK, val);
	}

	keys[old_bucket_max] = find_key_in_bucket(2, fm.slot_mask, 11111);
	enum tl_status val = fmap_intint_insert(&fm, keys[old_bucket_max], 60);

	for (size_t i = 0; i < old_bucket_max + 1; i++) {
		size_t hash = fmap_intint_fnv1a(keys[i]);
		size_t bucket = hash & fm.slot_mask;
		size_t slot = bucket * fm.bucket_max;
		size_t slot_i = 0;
		enum tl_status val = fmap_intint_probe_key(fm.nodes, fm.info, slot, fm.bucket_max, keys[i], &slot_i);
		TEST_ASSERT_EQUAL_INT(TLOK, val);
	}

	TEST_ASSERT_EQUAL_INT(TLOK, val);

	/* we expect that a grow will have occurred! */
	TEST_ASSERT_EQUAL_size_t(16, fm.num_buckets);
	TEST_ASSERT_EQUAL_size_t(old_bucket_max + 1, fm.size);
	TEST_ASSERT_EQUAL_size_t(fm.num_buckets * fm.bucket_max, fm.capacity);

	fmap_intint_deinit(&fm);
}

void test_insert_grow_into_beginning(void)
{
	struct fmap_intint fm;
	fmap_intint_init_all(&fm, 8, 70u);
	size_t old_bucket_max = fm.bucket_max;

	int keys[30];

	/**
	 * keep in mind that if the subsequent bucket will be 0, then the current bucket is also
	 * guaranteed to be 0
	 */
	for (size_t i = 1; i <= fm.bucket_max; i++) {
		keys[i - 1] = find_key_in_bucket(0, 15, i * 100000);
		enum tl_status val = fmap_intint_insert(&fm, keys[i - 1], i * 10);

		TEST_ASSERT_EQUAL_INT(TLOK, val);
	}

	TEST_ASSERT_EQUAL_size_t(8, fm.num_buckets);
	TEST_ASSERT_EQUAL_size_t(old_bucket_max, fm.size);

	keys[old_bucket_max] = find_key_in_bucket(0, 15, 11111);
	enum tl_status val = fmap_intint_insert(&fm, keys[old_bucket_max], 60);

	for (size_t i = 0; i < old_bucket_max + 1; i++) {
		size_t hash = fmap_intint_fnv1a(keys[i]);
		size_t bucket = hash & fm.slot_mask;
		size_t slot = bucket * fm.bucket_max;
		size_t slot_i = 0;
		enum tl_status ret = fmap_intint_probe_key(fm.nodes, fm.info, slot, fm.bucket_max, keys[i], &slot_i);
		TEST_ASSERT_EQUAL_INT(TLOK, ret);
	}

	TEST_ASSERT_EQUAL_INT(TLOK, val);

	/* we expect that a grow will have occurred! */
	TEST_ASSERT_EQUAL_size_t(16, fm.num_buckets);
	TEST_ASSERT_EQUAL_size_t(old_bucket_max + 1, fm.size);
	TEST_ASSERT_EQUAL_size_t(fm.num_buckets * fm.bucket_max, fm.capacity);

	fmap_intint_deinit(&fm);
}

void test_insert_grow_into_end(void)
{
	struct fmap_intint fm;
	fmap_intint_init_all(&fm, 8, 70u);
	size_t old_bucket_max = fm.bucket_max;

	int keys[30];

	/**
	 * keep in mind that if the subsequent bucket will be 15, then the earlier must also be the end
	 * bucket
	 */
	for (size_t i = 1; i <= fm.bucket_max; i++) {
		keys[i - 1] = find_key_in_bucket(15, 15, i * 100000);
		enum tl_status val = fmap_intint_insert(&fm, keys[i - 1], i * 10);

		TEST_ASSERT_EQUAL_INT(TLOK, val);
	}

	TEST_ASSERT_EQUAL_size_t(8, fm.num_buckets);
	TEST_ASSERT_EQUAL_size_t(old_bucket_max, fm.size);


	keys[old_bucket_max] = find_key_in_bucket(15, 15, 11111);
	enum tl_status val = fmap_intint_insert(&fm, keys[old_bucket_max], 60);

	for (size_t i = 0; i < old_bucket_max + 1; i++) {
		size_t hash = fmap_intint_fnv1a(keys[i]);
		size_t bucket = hash & fm.slot_mask;
		size_t slot = bucket * fm.bucket_max;
		size_t slot_i = 0;
		enum tl_status ret = fmap_intint_probe_key(fm.nodes, fm.info, slot, fm.bucket_max, keys[i], &slot_i);
		TEST_ASSERT_EQUAL_INT(TLOK, ret);
	}

	TEST_ASSERT_EQUAL_INT(TLOK, val);

	/* we expect that a grow will have occurred! */
	TEST_ASSERT_EQUAL_size_t(16, fm.num_buckets);
	TEST_ASSERT_EQUAL_size_t(old_bucket_max + 1, fm.size);
	TEST_ASSERT_EQUAL_size_t(fm.num_buckets * fm.bucket_max, fm.capacity);

	fmap_intint_deinit(&fm);
}

void test_insert_grow_size_bound(void)
{
	struct fmap_intint fm;
	fmap_intint_init_all(&fm, 8, 70u);

	size_t bound = (fm.num_buckets * fm.bucket_max * fm.load_factor) / 100;
	int bucket = 0;

	/**
	 * Distribute in to a guaranteed bucket up till the grow boundary
	 */
	for (int i = 0; i < bound; i++, bucket++) {
		if (bucket >= fm.num_buckets)
			bucket = 0;

		int key = find_key_in_bucket(bucket, fm.slot_mask, 12345 * i + 1);
		fmap_intint_insert(&fm, key, 10 * (i + 1));
	}

	TEST_ASSERT_EQUAL_size_t(8, fm.num_buckets);
	TEST_ASSERT_EQUAL_size_t(bound, fm.size);

	fmap_intint_deinit(&fm);
}

void test_insert_over_grow_bound(void)
{
	struct fmap_intint fm;
	fmap_intint_init_all(&fm, 8, 70u);

	size_t bound = (fm.num_buckets * fm.bucket_max * fm.load_factor) / 100;
	int bucket = 0;

	/**
	 * Distribute in to a guaranteed bucket up till the grow boundary
	 */
	for (int i = 0; i < bound; i++, bucket++) {
		if (bucket >= fm.num_buckets)
			bucket = 0;

		int key = find_key_in_bucket(bucket, fm.slot_mask, 12345 * i + 1);
		fmap_intint_insert(&fm, key, 10 * (i + 1));
	}

	TEST_ASSERT_EQUAL_size_t(8, fm.num_buckets);
	TEST_ASSERT_EQUAL_size_t(bound, fm.size);

	int key = find_key_in_bucket(bucket, fm.slot_mask, 12345 * (int)bound + 1);
	fmap_intint_add(&fm, key, 10 * ((int)bound + 1));

	TEST_ASSERT_EQUAL_size_t(16, fm.num_buckets);
	TEST_ASSERT_EQUAL_size_t(bound + 1u, fm.size);
	TEST_ASSERT_EQUAL_size_t(44, fm.load_max);

	fmap_intint_deinit(&fm);
}

void test_insert_overwrite_many(void)
{
	struct fmap_intint fm;
	fmap_intint_init_all(&fm, 8, 70u);

	int num = 2000;
	int* keys = generate_list(num, 1);
	for (int i = 0; i < num; i++) {
		TEST_ASSERT_EQUAL_INT(TLOK, fmap_intint_insert(&fm, keys[i], i + 1234));
	}

	TEST_ASSERT_EQUAL_size_t(num, fm.size);
	size_t capacity_check = fm.capacity;

	for (int i = 0; i < num; i++) {
		TEST_ASSERT_EQUAL_INT(TLOK, fmap_intint_insert(&fm, keys[i], i + 4321));
	}

	TEST_ASSERT_EQUAL_size_t(capacity_check, fm.capacity);
	TEST_ASSERT_EQUAL_size_t(num, fm.size);

	for (int i = 0; i < num; i++) {
		TEST_ASSERT_EQUAL_INT(i + 4321, fmap_intint_get(&fm, keys[i]));
	}

	tlfree(keys);
	fmap_intint_deinit(&fm);
}






/**********************************************************************************************************************
 * erase Tests
 **********************************************************************************************************************/
#ifndef TEST_TL_NO_ZERO_MEM

void test_erase_one(void)
{
	struct fmap_intint fm;
	fmap_intint_init_all(&fm, 8, 70u);

	int key = 98575;
	fmap_intint_add(&fm, key, 20);

	TEST_ASSERT_EQUAL_INT(TLOK, fmap_intint_erase(&fm, key));
	TEST_ASSERT_EQUAL_size_t(0, fm.size);

	const size_t hash = fmap_intint_fnv1a(key);
	const size_t slot = (hash & fm.slot_mask) * fm.bucket_max;

	int tester = 0;
	memset(&tester, TL_INIT_VAL, sizeof(int));
	TEST_ASSERT_EQUAL_INT(TL_MAPSS_EMPTY, fm.info[slot]);
	TEST_ASSERT_EQUAL_INT(tester, fm.nodes[slot].key);
	TEST_ASSERT_EQUAL_INT(tester, fm.nodes[slot].value);

	fmap_intint_deinit(&fm);
}

void test_erase_first(void)
{
	struct fmap_intint fm;
	fmap_intint_init_all(&fm, 8, 70u);

	int key = find_key_in_bucket(0, fm.slot_mask, 19385);
	fmap_intint_add(&fm, key, 21039);

	TEST_ASSERT_EQUAL_INT(TL_MAPSS_OCCUPIED, fm.info[0]);
	TEST_ASSERT_EQUAL_INT(TLOK, fmap_intint_erase(&fm, key));

	int tester = 0;
	memset(&tester, TL_INIT_VAL, sizeof(int));
	TEST_ASSERT_EQUAL_INT(TL_MAPSS_EMPTY, fm.info[0]);
	TEST_ASSERT_EQUAL_INT(tester, fm.nodes[0].key);
	TEST_ASSERT_EQUAL_INT(tester, fm.nodes[0].value);

	fmap_intint_deinit(&fm);
}

void test_erase_in_end_bucket(void)
{
	struct fmap_intint fm;
	fmap_intint_init_all(&fm, 8, 70u);

	int key = find_key_in_bucket(7, fm.slot_mask, 19385);
	fmap_intint_add(&fm, key, 21039);

	TEST_ASSERT_EQUAL_INT(TL_MAPSS_OCCUPIED, fm.info[fm.bucket_max * 7]);
	TEST_ASSERT_EQUAL_INT(TLOK, fmap_intint_erase(&fm, key));

	int tester = 0;
	memset(&tester, TL_INIT_VAL, sizeof(int));
	TEST_ASSERT_EQUAL_INT(TL_MAPSS_EMPTY, fm.info[fm.bucket_max * 7]);
	TEST_ASSERT_EQUAL_INT(tester, fm.nodes[fm.bucket_max * 7].key);
	TEST_ASSERT_EQUAL_INT(tester, fm.nodes[fm.bucket_max * 7].value);

	fmap_intint_deinit(&fm);
}

void test_erase_last_in_bucket(void)
{
	struct fmap_intint fm;
	fmap_intint_init_all(&fm, 8, 70u);

	int keys[fm.bucket_max];
	for (int i = 0; i < fm.bucket_max; i++) {
		keys[i] = find_key_in_bucket(0, fm.slot_mask, 19385 + (i * 1000));
		fmap_intint_add(&fm, keys[i], 21039 + keys[i]);
	}

	TEST_ASSERT_EQUAL_INT(TL_MAPSS_COLLIDED, fm.info[fm.bucket_max - 1]);
	TEST_ASSERT_EQUAL_INT(TLOK, fmap_intint_erase(&fm, keys[fm.bucket_max - 1]));

	int tester = 0;
	memset(&tester, TL_INIT_VAL, sizeof(int));
	TEST_ASSERT_EQUAL_INT(TL_MAPSS_EMPTY, fm.info[fm.bucket_max - 1]);
	TEST_ASSERT_EQUAL_INT(tester, fm.nodes[fm.bucket_max - 1].key);
	TEST_ASSERT_EQUAL_INT(tester, fm.nodes[fm.bucket_max - 1].value);
	TEST_ASSERT_EQUAL_size_t(fm.bucket_max - 1u, fm.size);

	fmap_intint_deinit(&fm);
}

void test_erase_last_in_last_bucket(void)
{
	struct fmap_intint fm;
	fmap_intint_init_all(&fm, 8, 70u);

	int keys[fm.bucket_max];
	for (int i = 0; i < fm.bucket_max; i++) {
		keys[i] = find_key_in_bucket(7, fm.slot_mask, 19385 + (i * 1000));
		fmap_intint_add(&fm, keys[i], 21039 + keys[i]);
	}

	size_t end_pos = (fm.bucket_max * 7u) + (fm.bucket_max - 1);
	TEST_ASSERT_EQUAL_INT(TL_MAPSS_COLLIDED, fm.info[end_pos]);
	TEST_ASSERT_EQUAL_INT(TLOK, fmap_intint_erase(&fm, keys[fm.bucket_max - 1]));

	int tester = 0;
	memset(&tester, TL_INIT_VAL, sizeof(int));
	TEST_ASSERT_EQUAL_INT(TL_MAPSS_EMPTY, fm.info[end_pos]);
	TEST_ASSERT_EQUAL_INT(tester, fm.nodes[end_pos].key);
	TEST_ASSERT_EQUAL_INT(tester, fm.nodes[end_pos].value);
	TEST_ASSERT_EQUAL_size_t(fm.bucket_max - 1u, fm.size);

	fmap_intint_deinit(&fm);
}

void test_erase_only_hits_requested_node(void)
{
	struct fmap_intint fm;
	fmap_intint_init_all(&fm, 8, 70u);

	int keys[fm.bucket_max + 1];
	for (int i = 0; i < fm.bucket_max; i++) {
		keys[i] = find_key_in_bucket(0, fm.slot_mask, 19385 + (i * 1000));
		fmap_intint_add(&fm, keys[i], 21039 + keys[i]);
	}

	keys[fm.bucket_max] = find_key_in_bucket(1, fm.slot_mask, 294747);
	fmap_intint_add(&fm, keys[fm.bucket_max], 11309);

	TEST_ASSERT_EQUAL_INT(TL_MAPSS_COLLIDED, fm.info[fm.bucket_max - 1]);
	TEST_ASSERT_EQUAL_INT(TLOK, fmap_intint_erase(&fm, keys[fm.bucket_max - 1]));

	int tester = 0;
	memset(&tester, TL_INIT_VAL, sizeof(int));
	TEST_ASSERT_EQUAL_INT(TL_MAPSS_EMPTY, fm.info[fm.bucket_max - 1]);
	TEST_ASSERT_EQUAL_INT(tester, fm.nodes[fm.bucket_max - 1].key);
	TEST_ASSERT_EQUAL_INT(tester, fm.nodes[fm.bucket_max - 1].value);
	TEST_ASSERT_EQUAL_size_t(fm.bucket_max, fm.size);

	TEST_ASSERT_EQUAL_INT(TL_MAPSS_OCCUPIED, fm.info[fm.bucket_max]);
	TEST_ASSERT_EQUAL_INT(keys[fm.bucket_max], fm.nodes[fm.bucket_max].key);
	TEST_ASSERT_EQUAL_INT(11309, fm.nodes[fm.bucket_max].value);

	fmap_intint_deinit(&fm);
}

void test_erase_all(void)
{
	struct fmap_intint fm;
	fmap_intint_init_all(&fm, 8, 70);

	int count = 1000;
	int* keys = generate_list(count, 1);
	for (int i = 0; i < count; i++) {
		fmap_intint_add(&fm, keys[i], 147 + keys[i]);
	}

	TEST_ASSERT_EQUAL_size_t(count, fm.size);

	for (int i = 0; i < count; i++) {
		TEST_ASSERT_EQUAL_INT(TLOK, fmap_intint_erase(&fm, keys[i]));
	}

	TEST_ASSERT_EQUAL_size_t(0u, fm.size);

	for (size_t i = 0; i < fm.capacity; i++) {
		TEST_ASSERT_EQUAL_INT(TL_MAPSS_EMPTY, fm.info[i]);
	}

	tlfree(keys);
	fmap_intint_deinit(&fm);
}






/**********************************************************************************************************************
 * remove Tests
 **********************************************************************************************************************/

void test_remove_one(void)
{
	struct fmap_intint fm;
	fmap_intint_init_all(&fm, 8, 70u);

	int key = 98575;
	fmap_intint_add(&fm, key, 20);

	int value = 0;
	TEST_ASSERT_EQUAL_INT(TLOK, fmap_intint_remove(&fm, key, &value));
	TEST_ASSERT_EQUAL_size_t(0, fm.size);
	TEST_ASSERT_EQUAL_INT(20, value);

	const size_t hash = fmap_intint_fnv1a(key);
	const size_t slot = (hash & fm.slot_mask) * fm.bucket_max;

	int tester = 0;
	memset(&tester, TL_INIT_VAL, sizeof(int));
	TEST_ASSERT_EQUAL_INT(TL_MAPSS_EMPTY, fm.info[slot]);
	TEST_ASSERT_EQUAL_INT(tester, fm.nodes[slot].key);
	TEST_ASSERT_EQUAL_INT(tester, fm.nodes[slot].value);

	fmap_intint_deinit(&fm);
}

void test_remove_first(void)
{
	struct fmap_intint fm;
	fmap_intint_init_all(&fm, 8, 70u);

	int key = find_key_in_bucket(0, fm.slot_mask, 19385);
	fmap_intint_add(&fm, key, 21039);

	int value = 0;
	TEST_ASSERT_EQUAL_INT(TL_MAPSS_OCCUPIED, fm.info[0]);
	TEST_ASSERT_EQUAL_INT(TLOK, fmap_intint_remove(&fm, key, &value));
	TEST_ASSERT_EQUAL_INT(21039, value);

	int tester = 0;
	memset(&tester, TL_INIT_VAL, sizeof(int));
	TEST_ASSERT_EQUAL_INT(TL_MAPSS_EMPTY, fm.info[0]);
	TEST_ASSERT_EQUAL_INT(tester, fm.nodes[0].key);
	TEST_ASSERT_EQUAL_INT(tester, fm.nodes[0].value);

	fmap_intint_deinit(&fm);
}

void test_remove_in_end_bucket(void)
{
	struct fmap_intint fm;
	fmap_intint_init_all(&fm, 8, 70u);

	int key = find_key_in_bucket(7, fm.slot_mask, 19385);
	fmap_intint_add(&fm, key, 21039);

	int value = 0;
	TEST_ASSERT_EQUAL_INT(TL_MAPSS_OCCUPIED, fm.info[fm.bucket_max * 7]);
	TEST_ASSERT_EQUAL_INT(TLOK, fmap_intint_remove(&fm, key, &value));
	TEST_ASSERT_EQUAL_INT(21039, value);

	int tester = 0;
	memset(&tester, TL_INIT_VAL, sizeof(int));
	TEST_ASSERT_EQUAL_INT(TL_MAPSS_EMPTY, fm.info[fm.bucket_max * 7]);
	TEST_ASSERT_EQUAL_INT(tester, fm.nodes[fm.bucket_max * 7].key);
	TEST_ASSERT_EQUAL_INT(tester, fm.nodes[fm.bucket_max * 7].value);

	fmap_intint_deinit(&fm);
}

void test_remove_last_in_bucket(void)
{
	struct fmap_intint fm;
	fmap_intint_init_all(&fm, 8, 70u);

	int keys[fm.bucket_max];
	for (int i = 0; i < fm.bucket_max; i++) {
		keys[i] = find_key_in_bucket(0, fm.slot_mask, 19385 + (i * 1000));
		fmap_intint_add(&fm, keys[i], 21039 + keys[i]);
	}

	int value = 0;
	TEST_ASSERT_EQUAL_INT(TL_MAPSS_COLLIDED, fm.info[fm.bucket_max - 1]);
	TEST_ASSERT_EQUAL_INT(TLOK, fmap_intint_remove(&fm, keys[fm.bucket_max - 1], &value));
	TEST_ASSERT_EQUAL_INT(21039 + keys[fm.bucket_max - 1], value);

	int tester = 0;
	memset(&tester, TL_INIT_VAL, sizeof(int));
	TEST_ASSERT_EQUAL_INT(TL_MAPSS_EMPTY, fm.info[fm.bucket_max - 1]);
	TEST_ASSERT_EQUAL_INT(tester, fm.nodes[fm.bucket_max - 1].key);
	TEST_ASSERT_EQUAL_INT(tester, fm.nodes[fm.bucket_max - 1].value);
	TEST_ASSERT_EQUAL_size_t(fm.bucket_max - 1u, fm.size);

	fmap_intint_deinit(&fm);
}

void test_remove_last_in_last_bucket(void)
{
	struct fmap_intint fm;
	fmap_intint_init_all(&fm, 8, 70u);

	int keys[fm.bucket_max];
	for (int i = 0; i < fm.bucket_max; i++) {
		keys[i] = find_key_in_bucket(7, fm.slot_mask, 19385 + (i * 1000));
		fmap_intint_add(&fm, keys[i], 21039 + keys[i]);
	}

	int value = 0;
	size_t end_pos = (fm.bucket_max * 7u) + (fm.bucket_max - 1);
	TEST_ASSERT_EQUAL_INT(TL_MAPSS_COLLIDED, fm.info[end_pos]);
	TEST_ASSERT_EQUAL_INT(TLOK, fmap_intint_remove(&fm, keys[fm.bucket_max - 1], &value));
	TEST_ASSERT_EQUAL_INT(21039 + keys[fm.bucket_max - 1], value);

	int tester = 0;
	memset(&tester, TL_INIT_VAL, sizeof(int));
	TEST_ASSERT_EQUAL_INT(TL_MAPSS_EMPTY, fm.info[end_pos]);
	TEST_ASSERT_EQUAL_INT(tester, fm.nodes[end_pos].key);
	TEST_ASSERT_EQUAL_INT(tester, fm.nodes[end_pos].value);
	TEST_ASSERT_EQUAL_size_t(fm.bucket_max - 1u, fm.size);

	fmap_intint_deinit(&fm);
}

void test_remove_only_hits_requested_node(void)
{
	struct fmap_intint fm;
	fmap_intint_init_all(&fm, 8, 70u);

	int keys[fm.bucket_max + 1];
	for (int i = 0; i < fm.bucket_max; i++) {
		keys[i] = find_key_in_bucket(0, fm.slot_mask, 19385 + (i * 1000));
		fmap_intint_add(&fm, keys[i], 21039 + keys[i]);
	}

	keys[fm.bucket_max] = find_key_in_bucket(1, fm.slot_mask, 294747);
	fmap_intint_add(&fm, keys[fm.bucket_max], 11309);

	int value = 0;
	TEST_ASSERT_EQUAL_INT(TL_MAPSS_COLLIDED, fm.info[fm.bucket_max - 1]);
	TEST_ASSERT_EQUAL_INT(TLOK, fmap_intint_remove(&fm, keys[fm.bucket_max - 1], &value));
	TEST_ASSERT_EQUAL_INT(21039 + keys[fm.bucket_max - 1], value);

	int tester = 0;
	memset(&tester, TL_INIT_VAL, sizeof(int));
	TEST_ASSERT_EQUAL_INT(TL_MAPSS_EMPTY, fm.info[fm.bucket_max - 1]);
	TEST_ASSERT_EQUAL_INT(tester, fm.nodes[fm.bucket_max - 1].key);
	TEST_ASSERT_EQUAL_INT(tester, fm.nodes[fm.bucket_max - 1].value);
	TEST_ASSERT_EQUAL_size_t(fm.bucket_max, fm.size);

	TEST_ASSERT_EQUAL_INT(TL_MAPSS_OCCUPIED, fm.info[fm.bucket_max]);
	TEST_ASSERT_EQUAL_INT(keys[fm.bucket_max], fm.nodes[fm.bucket_max].key);
	TEST_ASSERT_EQUAL_INT(11309, fm.nodes[fm.bucket_max].value);

	fmap_intint_deinit(&fm);
}

void test_remove_all(void)
{
	struct fmap_intint fm;
	fmap_intint_init_all(&fm, 8, 70);

	int count = 1000;
	int* keys = generate_list(count, 1);
	for (int i = 0; i < count; i++) {
		fmap_intint_add(&fm, keys[i], 147 + keys[i]);
	}

	TEST_ASSERT_EQUAL_size_t(count, fm.size);

	int value = 0;
	for (int i = 0; i < count; i++) {
		TEST_ASSERT_EQUAL_INT(TLOK, fmap_intint_remove(&fm, keys[i], &value));
		TEST_ASSERT_EQUAL_INT(147 + keys[i], value);
	}

	TEST_ASSERT_EQUAL_size_t(0u, fm.size);

	for (size_t i = 0; i < fm.capacity; i++) {
		TEST_ASSERT_EQUAL_INT(TL_MAPSS_EMPTY, fm.info[i]);
	}

	tlfree(keys);
	fmap_intint_deinit(&fm);
}

#endif



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
	RUN_TEST(test_grow_with_one_ele);

	RUN_TEST(test_add_one);
	RUN_TEST(test_add_begin);
	RUN_TEST(test_add_end);
	RUN_TEST(test_add_two_diff_buckets);
	RUN_TEST(test_add_two_same_bucket);
	RUN_TEST(test_add_two_same_bucket_begin);
	RUN_TEST(test_add_two_same_key);
	RUN_TEST(test_add_fill_bucket);
	RUN_TEST(test_add_fill_bucket_then_diff_bucket);
	RUN_TEST(test_add_grow_by_full_bucket);
	RUN_TEST(test_add_grow_into_beginning);
	RUN_TEST(test_add_grow_into_end);
	RUN_TEST(test_add_grow_size_bound);
	RUN_TEST(test_add_over_grow_bound);

	RUN_TEST(test_get_one);
	RUN_TEST(test_get_begin);
	RUN_TEST(test_get_last_bucket);
	RUN_TEST(test_get_end_of_begin);
	RUN_TEST(test_get_end_of_end);
	RUN_TEST(test_get_after_grow);
	RUN_TEST(test_get_after_many_grow);

	RUN_TEST(test_try_get_one);
	RUN_TEST(test_try_get_begin);
	RUN_TEST(test_try_get_missing);
	RUN_TEST(test_try_get_last_bucket);
	RUN_TEST(test_try_get_end_of_begin);
	RUN_TEST(test_try_get_end_of_end);
	RUN_TEST(test_try_get_after_grow);
	RUN_TEST(test_try_get_after_many_grow);

	RUN_TEST(test_insert_one);
	RUN_TEST(test_insert_begin);
	RUN_TEST(test_insert_end);
	RUN_TEST(test_insert_two_diff_buckets);
	RUN_TEST(test_insert_two_same_bucket);
	RUN_TEST(test_insert_two_same_bucket_begin);
	RUN_TEST(test_insert_two_same_key);
	RUN_TEST(test_insert_fill_bucket);
	RUN_TEST(test_insert_fill_bucket_then_diff_bucket);
	RUN_TEST(test_insert_grow_by_full_bucket);
	RUN_TEST(test_insert_grow_into_beginning);
	RUN_TEST(test_insert_grow_into_end);
	RUN_TEST(test_insert_grow_size_bound);
	RUN_TEST(test_insert_over_grow_bound);
	RUN_TEST(test_insert_overwrite_many);

	RUN_TEST(test_erase_one);
	RUN_TEST(test_erase_first);
	RUN_TEST(test_erase_last_in_bucket);
	RUN_TEST(test_erase_in_end_bucket);
	RUN_TEST(test_erase_last_in_last_bucket);
	RUN_TEST(test_erase_only_hits_requested_node);
	RUN_TEST(test_erase_all);

	RUN_TEST(test_remove_one);
	RUN_TEST(test_remove_first);
	RUN_TEST(test_remove_in_end_bucket);
	RUN_TEST(test_remove_last_in_bucket);
	RUN_TEST(test_remove_last_in_last_bucket);
	RUN_TEST(test_remove_only_hits_requested_node);
	RUN_TEST(test_remove_all);


	return UNITY_END();
}