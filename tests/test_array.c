#include "unity.h"

#include <stdlib.h>
#include <string.h>

#define TL_T int

#include "array.h"

void setUp(void)
{}

void tearDown(void)
{}

void test_generated_type(void)
{
	struct array_int array = {0};

	TEST_ASSERT_EQUAL_INT(array.size, 0);
	TEST_ASSERT_EQUAL_INT(array.capacity, 0);
	TEST_ASSERT_EQUAL_FLOAT(array.grow_factor, 0.0f);
	TEST_ASSERT_EQUAL_PTR(array.data, NULL);
}

void test_deinit(void)
{
	struct array_int array = {0};
	array.size = 1;
	array.data = tlmalloc(10 * sizeof(int));
	array_int_deinit(&array);

	TEST_ASSERT_EQUAL_INT(array.size, 0);
	TEST_ASSERT_EQUAL_PTR(array.data, NULL);
}

void test_init_no_arg(void)
{
	struct array_int array;

	TEST_ASSERT_EQUAL_INT(array_int_init(&array), 1);
	TEST_ASSERT_EQUAL_INT(array.size, 0);
	TEST_ASSERT_EQUAL_INT(array.capacity, 20);
	TEST_ASSERT_EQUAL_FLOAT(array.grow_factor, 2.0f);
	TEST_ASSERT_NOT_NULL(array.data);

	array_int_deinit(&array);
}

void test_delete(void)
{
	struct array_int* array = tlmalloc(1 * sizeof(struct array_int));
	array_int_init(array);
	array_int_delete(&array);

	TEST_ASSERT_EQUAL_PTR(array, NULL);
}

void test_new(void)
{
	struct array_int* array = array_int_new();

	TEST_ASSERT_NOT_NULL(array);
	TEST_ASSERT_EQUAL_INT(array->capacity, 20);
	TEST_ASSERT_EQUAL_FLOAT(array->grow_factor, 2.0f);
	TEST_ASSERT_EQUAL_INT(array->size, 0);
	TEST_ASSERT_NOT_NULL(array->data);

	array_int_delete(&array);
}

void test_empty_grow(void)
{
	struct array_int array;
	array_int_init(&array);

	TEST_ASSERT_EQUAL_INT(array_int_grow(&array), TLOK);

	TEST_ASSERT_EQUAL_INT(array.capacity, 40);
	TEST_ASSERT_EQUAL_FLOAT(array.grow_factor, 2.0f);
	TEST_ASSERT_EQUAL_INT(array.size, 0);
	TEST_ASSERT_NOT_NULL(array.data);

	array_int_deinit(&array);
}


/**********************************************************************************************************************
 * append Tests
 **********************************************************************************************************************/

void test_append_one(void)
{
	struct array_int array;
	array_int_init(&array);

	TEST_ASSERT_EQUAL_INT(array_int_append(&array, 10), TLOK);

	TEST_ASSERT_EQUAL_INT(array.data[0], 10);
	TEST_ASSERT_EQUAL_INT(array.size, 1);
	TEST_ASSERT_EQUAL_INT(array.capacity, 20);

	array_int_deinit(&array);
}

void test_append_two(void)
{
	struct array_int array;
	array_int_init(&array);

	array_int_append(&array, 10);
	array_int_append(&array, 20);

	TEST_ASSERT_EQUAL_INT(array.data[0], 10);
	TEST_ASSERT_EQUAL_INT(array.data[1], 20);
	TEST_ASSERT_EQUAL_INT(array.size, 2);
	TEST_ASSERT_EQUAL_INT(array.capacity, 20);

	array_int_deinit(&array);
}

void test_append_pre_grow_bound(void)
{
	struct array_int array;
	array_int_init_all(&array, 4, 2.0f);

	array_int_append(&array, 10);
	array_int_append(&array, 20);
	array_int_append(&array, 30);
	array_int_append(&array, 40);

	TEST_ASSERT_EQUAL_INT(array.data[0], 10);
	TEST_ASSERT_EQUAL_INT(array.data[1], 20);
	TEST_ASSERT_EQUAL_INT(array.data[2], 30);
	TEST_ASSERT_EQUAL_INT(array.data[3], 40);
	TEST_ASSERT_EQUAL_INT(array.size, 4);
	TEST_ASSERT_EQUAL_INT(array.capacity, 4);

	array_int_deinit(&array);
}

void test_append_over_grow_bound(void)
{
	struct array_int array;
	array_int_init_all(&array, 4, 2.0f);

	array_int_append(&array, 10);
	array_int_append(&array, 20);
	array_int_append(&array, 30);
	array_int_append(&array, 40);
	array_int_append(&array, 50);

	TEST_ASSERT_EQUAL_INT(array.data[0], 10);
	TEST_ASSERT_EQUAL_INT(array.data[1], 20);
	TEST_ASSERT_EQUAL_INT(array.data[2], 30);
	TEST_ASSERT_EQUAL_INT(array.data[3], 40);
	TEST_ASSERT_EQUAL_INT(array.data[4], 50);
	TEST_ASSERT_EQUAL_INT(array.size, 5);
	TEST_ASSERT_EQUAL_INT(array.capacity, 8);

	array_int_deinit(&array);
}



/**********************************************************************************************************************
 * push Tests
 **********************************************************************************************************************/

void test_push_one(void)
{
	struct array_int array;
	array_int_init_all(&array, 4, 2.0f);

	array_int_push(&array, 10);

	TEST_ASSERT_EQUAL_INT(array.data[0], 10);
	TEST_ASSERT_EQUAL_INT(array.size, 1);
	TEST_ASSERT_EQUAL_INT(array.capacity, 4);

	array_int_deinit(&array);
}

void test_push_two(void)
{
	struct array_int array;
	array_int_init_all(&array, 4, 2.0f);

	array_int_push(&array, 10);
	array_int_push(&array, 20);

	TEST_ASSERT_EQUAL_INT(array.data[1], 10);
	TEST_ASSERT_EQUAL_INT(array.data[0], 20);
	TEST_ASSERT_EQUAL_INT(array.size, 2);
	TEST_ASSERT_EQUAL_INT(array.capacity, 4);

	array_int_deinit(&array);
}

void test_push_pre_grow_bound(void)
{
	struct array_int array;
	array_int_init_all(&array, 4, 2.0f);

	array_int_push(&array, 10);
	array_int_push(&array, 20);
	array_int_push(&array, 30);
	array_int_push(&array, 40);

	TEST_ASSERT_EQUAL_INT(array.data[3], 10);
	TEST_ASSERT_EQUAL_INT(array.data[2], 20);
	TEST_ASSERT_EQUAL_INT(array.data[1], 30);
	TEST_ASSERT_EQUAL_INT(array.data[0], 40);
	TEST_ASSERT_EQUAL_INT(array.size, 4);
	TEST_ASSERT_EQUAL_INT(array.capacity, 4);

	array_int_deinit(&array);
}

void test_push_over_grow_bound(void)
{
	struct array_int array;
	array_int_init_all(&array, 4, 2.0f);

	array_int_push(&array, 10);
	array_int_push(&array, 20);
	array_int_push(&array, 30);
	array_int_push(&array, 40);
	array_int_push(&array, 50);

	TEST_ASSERT_EQUAL_INT(array.data[4], 10);
	TEST_ASSERT_EQUAL_INT(array.data[3], 20);
	TEST_ASSERT_EQUAL_INT(array.data[2], 30);
	TEST_ASSERT_EQUAL_INT(array.data[1], 40);
	TEST_ASSERT_EQUAL_INT(array.data[0], 50);
	TEST_ASSERT_EQUAL_INT(array.size, 5);
	TEST_ASSERT_EQUAL_INT(array.capacity, 8);

	array_int_deinit(&array);
}






/**********************************************************************************************************************
 * insert Tests
 **********************************************************************************************************************/

void test_insert_one(void)
{
	struct array_int array;
	array_int_init_all(&array, 4, 2.0f);

	TEST_ASSERT_EQUAL_INT(array_int_insert(&array, 0, 10), TLOK);

	TEST_ASSERT_EQUAL_INT(array.data[0], 10);
	TEST_ASSERT_EQUAL_INT(array.size, 1);
	TEST_ASSERT_EQUAL_INT(array.capacity, 4);

	array_int_deinit(&array);
}

void test_insert_two(void)
{
	struct array_int array;
	array_int_init_all(&array, 4, 2.0f);

	array_int_insert(&array, 0, 10);
	array_int_insert(&array, 0, 20);

	TEST_ASSERT_EQUAL_INT(array.data[1], 10);
	TEST_ASSERT_EQUAL_INT(array.data[0], 20);
	TEST_ASSERT_EQUAL_INT(array.size, 2);
	TEST_ASSERT_EQUAL_INT(array.capacity, 4);

	array_int_deinit(&array);
}

void test_insert_pre_grow_bound(void)
{
	struct array_int array;
	array_int_init_all(&array, 4, 2.0f);

	array_int_insert(&array, 0, 10);
	array_int_insert(&array, 0, 20);
	array_int_insert(&array, 0, 30);
	array_int_insert(&array, 0, 40);

	TEST_ASSERT_EQUAL_INT(array.data[3], 10);
	TEST_ASSERT_EQUAL_INT(array.data[2], 20);
	TEST_ASSERT_EQUAL_INT(array.data[1], 30);
	TEST_ASSERT_EQUAL_INT(array.data[0], 40);
	TEST_ASSERT_EQUAL_INT(array.size, 4);
	TEST_ASSERT_EQUAL_INT(array.capacity, 4);

	array_int_deinit(&array);
}

void test_insert_over_grow_bound(void)
{
	struct array_int array;
	array_int_init_all(&array, 4, 2.0f);

	array_int_insert(&array, 0, 10);
	array_int_insert(&array, 0, 20);
	array_int_insert(&array, 0, 30);
	array_int_insert(&array, 0, 40);
	array_int_insert(&array, 0, 50);

	TEST_ASSERT_EQUAL_INT(array.data[4], 10);
	TEST_ASSERT_EQUAL_INT(array.data[3], 20);
	TEST_ASSERT_EQUAL_INT(array.data[2], 30);
	TEST_ASSERT_EQUAL_INT(array.data[1], 40);
	TEST_ASSERT_EQUAL_INT(array.data[0], 50);
	TEST_ASSERT_EQUAL_INT(array.size, 5);
	TEST_ASSERT_EQUAL_INT(array.capacity, 8);

	array_int_deinit(&array);
}

void test_insert_end_pre_grow_bound(void)
{
	struct array_int array;
	array_int_init_all(&array, 4, 2.0f);

	array_int_insert(&array, 0, 10);
	array_int_insert(&array, 0, 20);
	array_int_insert(&array, 2, 30);

	TEST_ASSERT_EQUAL_INT(array.data[1], 10);
	TEST_ASSERT_EQUAL_INT(array.data[0], 20);
	TEST_ASSERT_EQUAL_INT(array.data[2], 30);
	TEST_ASSERT_EQUAL_INT(array.size, 3);
	TEST_ASSERT_EQUAL_INT(array.capacity, 4);

	array_int_deinit(&array);
}

void test_insert_end_over_grow_bound(void)
{
	struct array_int array;
	array_int_init_all(&array, 4, 2.0f);

	array_int_insert(&array, 0, 10);
	array_int_insert(&array, 0, 20);
	array_int_insert(&array, 0, 30);
	array_int_insert(&array, 0, 40);
	array_int_insert(&array, 4, 50);

	TEST_ASSERT_EQUAL_INT(array.data[3], 10);
	TEST_ASSERT_EQUAL_INT(array.data[2], 20);
	TEST_ASSERT_EQUAL_INT(array.data[1], 30);
	TEST_ASSERT_EQUAL_INT(array.data[0], 40);
	TEST_ASSERT_EQUAL_INT(array.data[4], 50);
	TEST_ASSERT_EQUAL_INT(array.size, 5);
	TEST_ASSERT_EQUAL_INT(array.capacity, 8);

	array_int_deinit(&array);
}

void test_insert_middle_pre_grow_bound(void)
{
	struct array_int array;
	array_int_init_all(&array, 4, 2.0f);

	array_int_append(&array, 10);
	array_int_append(&array, 20);
	array_int_append(&array, 30);
	array_int_insert(&array, 2, 40);

	TEST_ASSERT_EQUAL_INT(array.data[0], 10);
	TEST_ASSERT_EQUAL_INT(array.data[1], 20);
	TEST_ASSERT_EQUAL_INT(array.data[2], 40);
	TEST_ASSERT_EQUAL_INT(array.data[3], 30);
	TEST_ASSERT_EQUAL_INT(array.size, 4);
	TEST_ASSERT_EQUAL_INT(array.capacity, 4);

	array_int_deinit(&array);
}

void test_insert_middle_over_grow_bound(void)
{
	struct array_int array;
	array_int_init_all(&array, 4, 2.0f);

	array_int_append(&array, 10);
	array_int_append(&array, 20);
	array_int_append(&array, 30);
	array_int_append(&array, 40);
	TEST_ASSERT_EQUAL_INT(array_int_insert(&array, 2, 50), TLOK);

	TEST_ASSERT_EQUAL_INT(array.data[0], 10);
	TEST_ASSERT_EQUAL_INT(array.data[1], 20);
	TEST_ASSERT_EQUAL_INT(array.data[3], 30);
	TEST_ASSERT_EQUAL_INT(array.data[4], 40);
	TEST_ASSERT_EQUAL_INT(array.data[2], 50);
	TEST_ASSERT_EQUAL_INT(array.size, 5);
	TEST_ASSERT_EQUAL_INT(array.capacity, 8);

	array_int_deinit(&array);
}





/**********************************************************************************************************************
 * replace Tests
 **********************************************************************************************************************/

void test_replace_one(void)
{
	struct array_int array;
	array_int_init(&array);

	array_int_insert(&array, 0, 10);
	array_int_replace(&array, 0, 50);

	TEST_ASSERT_EQUAL_INT(array.data[0], 50);
	TEST_ASSERT_EQUAL_INT(array.size, 1);

	array_int_deinit(&array);
}





/**********************************************************************************************************************
 * exchange Tests
 **********************************************************************************************************************/

void test_exchange_one(void)
{
	struct array_int array;
	array_int_init(&array);

	array_int_insert(&array, 0, 10);
	int value = array_int_exchange(&array, 0, 50);

	TEST_ASSERT_EQUAL_INT(value, 10);
	TEST_ASSERT_EQUAL_INT(array.data[0], 50);
	TEST_ASSERT_EQUAL_INT(array.size, 1);

	array_int_deinit(&array);
}





/**********************************************************************************************************************
 * erase Tests
 **********************************************************************************************************************/

void test_erase_one(void)
{
	struct array_int array;
	array_int_init(&array);

	array_int_append(&array, 10);
	array_int_append(&array, 20);
	array_int_append(&array, 30);
	array_int_append(&array, 40);
	array_int_erase(&array, 2);

	int check = 0;
	tlmemset(&check, TL_INIT_VAL, 1 * sizeof(int));

	TEST_ASSERT_EQUAL_INT(array.data[0], 10);
	TEST_ASSERT_EQUAL_INT(array.data[1], 20);
	TEST_ASSERT_EQUAL_INT(array.data[2], 40);
	TEST_ASSERT_EQUAL_INT(array.data[3], check);

	TEST_ASSERT_EQUAL_INT(array.size, 3);

	array_int_deinit(&array);
}

void test_erase_last(void)
{
	struct array_int array;
	array_int_init(&array);

	array_int_append(&array, 10);
	array_int_append(&array, 20);
	array_int_append(&array, 30);
	array_int_append(&array, 40);
	array_int_erase(&array, 3);

	int check = 0;
	tlmemset(&check, TL_INIT_VAL, 1 * sizeof(int));

	TEST_ASSERT_EQUAL_INT(array.data[0], 10);
	TEST_ASSERT_EQUAL_INT(array.data[1], 20);
	TEST_ASSERT_EQUAL_INT(array.data[2], 30);
	TEST_ASSERT_EQUAL_INT(array.data[3], check);

	TEST_ASSERT_EQUAL_INT(array.size, 3);

	array_int_deinit(&array);
}

void test_erase_first(void)
{
	struct array_int array;
	array_int_init(&array);

	array_int_append(&array, 10);
	array_int_append(&array, 20);
	array_int_append(&array, 30);
	array_int_append(&array, 40);
	array_int_erase(&array, 0);

	int check = 0;
	tlmemset(&check, TL_INIT_VAL, 1 * sizeof(int));

	TEST_ASSERT_EQUAL_INT(array.data[0], 20);
	TEST_ASSERT_EQUAL_INT(array.data[1], 30);
	TEST_ASSERT_EQUAL_INT(array.data[2], 40);
	TEST_ASSERT_EQUAL_INT(array.data[3], check);

	TEST_ASSERT_EQUAL_INT(array.size, 3);

	array_int_deinit(&array);
}





/**********************************************************************************************************************
 * remove Tests
 **********************************************************************************************************************/

void test_remove_one(void)
{
	struct array_int array;
	array_int_init(&array);

	array_int_append(&array, 10);
	array_int_append(&array, 20);
	array_int_append(&array, 30);
	array_int_append(&array, 40);
	TEST_ASSERT_EQUAL_INT(array_int_remove(&array, 2), 30);

	int check = 0;
	tlmemset(&check, TL_INIT_VAL, 1 * sizeof(int));

	TEST_ASSERT_EQUAL_INT(array.data[0], 10);
	TEST_ASSERT_EQUAL_INT(array.data[1], 20);
	TEST_ASSERT_EQUAL_INT(array.data[2], 40);
	TEST_ASSERT_EQUAL_INT(array.data[3], check);

	TEST_ASSERT_EQUAL_INT(array.size, 3);

	array_int_deinit(&array);
}

void test_remove_last(void)
{
	struct array_int array;
	array_int_init(&array);

	array_int_append(&array, 10);
	array_int_append(&array, 20);
	array_int_append(&array, 30);
	array_int_append(&array, 40);
	TEST_ASSERT_EQUAL_INT(array_int_remove(&array, 3), 40);

	int check = 0;
	tlmemset(&check, TL_INIT_VAL, 1 * sizeof(int));

	TEST_ASSERT_EQUAL_INT(array.data[0], 10);
	TEST_ASSERT_EQUAL_INT(array.data[1], 20);
	TEST_ASSERT_EQUAL_INT(array.data[2], 30);
	TEST_ASSERT_EQUAL_INT(array.data[3], check);

	TEST_ASSERT_EQUAL_INT(array.size, 3);

	array_int_deinit(&array);
}

void test_remove_first(void)
{
	struct array_int array;
	array_int_init(&array);

	array_int_append(&array, 10);
	array_int_append(&array, 20);
	array_int_append(&array, 30);
	array_int_append(&array, 40);
	TEST_ASSERT_EQUAL_INT(array_int_remove(&array, 0), 10);

	int check = 0;
	tlmemset(&check, TL_INIT_VAL, 1 * sizeof(int));

	TEST_ASSERT_EQUAL_INT(array.data[0], 20);
	TEST_ASSERT_EQUAL_INT(array.data[1], 30);
	TEST_ASSERT_EQUAL_INT(array.data[2], 40);
	TEST_ASSERT_EQUAL_INT(array.data[3], check);

	TEST_ASSERT_EQUAL_INT(array.size, 3);

	array_int_deinit(&array);
}





/**********************************************************************************************************************
 * shrink_to_fit, ensure_capacity and clear Tests
 **********************************************************************************************************************/

void test_shrink_to_fit(void)
{
	struct array_int array;
	array_int_init(&array);

	array_int_append(&array, 10);
	array_int_append(&array, 20);
	array_int_append(&array, 30);
	array_int_append(&array, 40);

	int ret = array_int_shrink_to_fit(&array);
	TEST_ASSERT_EQUAL_INT(ret, TLOK);
	TEST_ASSERT_EQUAL_INT(array.data[0], 10);
	TEST_ASSERT_EQUAL_INT(array.data[1], 20);
	TEST_ASSERT_EQUAL_INT(array.data[2], 30);
	TEST_ASSERT_EQUAL_INT(array.data[3], 40);
	//array.data[5] = 10;		/* Expect failure when run with valgrind */

	TEST_ASSERT_EQUAL_INT(array.size, 4);
	TEST_ASSERT_EQUAL_INT(array.capacity, 4);

	array_int_deinit(&array);
}

void test_ensure_capacity_below(void)
{
	struct array_int array;
	array_int_init(&array);

	TEST_ASSERT_EQUAL_INT(array_int_ensure_capacity(&array, 10), TLOK);

	TEST_ASSERT_EQUAL_INT(array.capacity, 20);

	array_int_deinit(&array);
}

void test_ensure_capacity_above(void)
{
	struct array_int array;
	array_int_init(&array);

	array.data[19] = 30;
	TEST_ASSERT_EQUAL_INT(array_int_ensure_capacity(&array, 30), TLOK);

	TEST_ASSERT_EQUAL_INT(array.data[19], 30);
	TEST_ASSERT_EQUAL_INT(array.capacity, 30);

	array_int_deinit(&array);
}

void test_clear(void)
{
	struct array_int array;
	array_int_init(&array);

	array_int_append(&array, 10);
	array_int_append(&array, 20);
	array_int_append(&array, 30);
	array_int_append(&array, 40);
	array_int_clear(&array);

	int check = 0;
	tlmemset(&check, TL_INIT_VAL, 1 * sizeof(int));

	TEST_ASSERT_EQUAL_INT(array.data[0], check);
	TEST_ASSERT_EQUAL_INT(array.data[1], check);
	TEST_ASSERT_EQUAL_INT(array.data[2], check);
	TEST_ASSERT_EQUAL_INT(array.data[3], check);

	TEST_ASSERT_EQUAL_INT(array.size, 0);

	array_int_deinit(&array);
}








int main(void)
{
	UNITY_BEGIN();

	RUN_TEST(test_generated_type);
	RUN_TEST(test_deinit);
	RUN_TEST(test_init_no_arg);
	RUN_TEST(test_delete);
	RUN_TEST(test_new);
	RUN_TEST(test_empty_grow);

	RUN_TEST(test_append_one);
	RUN_TEST(test_append_two);
	RUN_TEST(test_append_pre_grow_bound);
	RUN_TEST(test_append_over_grow_bound);

	RUN_TEST(test_push_one);
	RUN_TEST(test_push_two);
	RUN_TEST(test_push_pre_grow_bound);
	RUN_TEST(test_push_over_grow_bound);

	RUN_TEST(test_insert_one);
	RUN_TEST(test_insert_two);
	RUN_TEST(test_insert_pre_grow_bound);
	RUN_TEST(test_insert_over_grow_bound);
	RUN_TEST(test_insert_end_pre_grow_bound);
	RUN_TEST(test_insert_end_over_grow_bound);
	RUN_TEST(test_insert_middle_pre_grow_bound);
	RUN_TEST(test_insert_middle_over_grow_bound);

	RUN_TEST(test_replace_one);

	RUN_TEST(test_exchange_one);

	RUN_TEST(test_erase_one);
	RUN_TEST(test_erase_last);
	RUN_TEST(test_erase_first);

	RUN_TEST(test_remove_one);
	RUN_TEST(test_remove_last);
	RUN_TEST(test_remove_first);

	RUN_TEST(test_shrink_to_fit);
	RUN_TEST(test_ensure_capacity_below);
	RUN_TEST(test_ensure_capacity_above);
	RUN_TEST(test_clear);

	return UNITY_END();
}


