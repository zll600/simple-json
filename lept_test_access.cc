#include "lept_test.h"

static void test_access_null() {
  lept_value v;

  lept_set_string(&v, "a", 1);
  v.reset();
  EXPECT_EQ_INT(LEPT_NULL, lept_get_type(&v));
  v.reset();
}

static void test_access_boolean() {
  lept_value v;

  lept_set_string(&v, "a", 1);
  lept_set_boolean(&v, 1);
  EXPECT_TRUE(lept_get_boolean(&v));
  lept_set_boolean(&v, 0);
  EXPECT_FALSE(lept_get_boolean(&v));
  v.reset();
}

static void test_access_number() {
  lept_value v;

  lept_set_string(&v, "a", 1);
  lept_set_number(&v, 1234.5);
  EXPECT_EQ_DOUBLE(1234.5, lept_get_number(&v));
  v.reset();
}

static void test_access_string() {
  lept_value v;

  lept_set_string(&v, "", 0);
  EXPECT_EQ_STRING("", lept_get_string(&v), lept_get_string_length(&v));
  lept_set_string(&v, "Hello", 5);
  EXPECT_EQ_STRING("Hello", lept_get_string(&v), lept_get_string_length(&v));
  v.reset();
}

static void test_access_array() {
  lept_value a, e;
  size_t i, j;

  for (j = 0; j <= 5; j += 5) {
    lept_set_array(&a, j);
    EXPECT_EQ_SIZE_T(0, lept_get_array_size(&a));
    EXPECT_EQ_SIZE_T(j, lept_get_array_capacity(&a));
    for (i = 0; i < 10; i++) {
      lept_set_number(&e, i);
      lept_move(lept_pushback_array_element(&a), &e);
      e.reset();
    }

    EXPECT_EQ_SIZE_T(10, lept_get_array_size(&a));
    for (i = 0; i < 10; i++)
      EXPECT_EQ_DOUBLE((double)i,
                       lept_get_number(lept_get_array_element(&a, i)));
  }

  lept_popback_array_element(&a);
  EXPECT_EQ_SIZE_T(9, lept_get_array_size(&a));
  for (i = 0; i < 9; i++)
    EXPECT_EQ_DOUBLE((double)i, lept_get_number(lept_get_array_element(&a, i)));

  lept_erase_array_element(&a, 4, 0);
  EXPECT_EQ_SIZE_T(9, lept_get_array_size(&a));
  for (i = 0; i < 9; i++)
    EXPECT_EQ_DOUBLE((double)i, lept_get_number(lept_get_array_element(&a, i)));

  lept_erase_array_element(&a, 8, 1);
  EXPECT_EQ_SIZE_T(8, lept_get_array_size(&a));
  for (i = 0; i < 8; i++)
    EXPECT_EQ_DOUBLE((double)i, lept_get_number(lept_get_array_element(&a, i)));

  lept_erase_array_element(&a, 0, 2);
  EXPECT_EQ_SIZE_T(6, lept_get_array_size(&a));
  for (i = 0; i < 6; i++)
    EXPECT_EQ_DOUBLE((double)i + 2,
                     lept_get_number(lept_get_array_element(&a, i)));

  EXPECT_EQ_SIZE_T(6, lept_get_array_size(&a));
  for (i = 0; i < 6; i++)
    EXPECT_EQ_DOUBLE((double)i + 2,
                     lept_get_number(lept_get_array_element(&a, i)));

  EXPECT_TRUE(lept_get_array_capacity(&a) > 6);
  lept_shrink_array(&a);
  EXPECT_EQ_SIZE_T(6, lept_get_array_capacity(&a));
  EXPECT_EQ_SIZE_T(6, lept_get_array_size(&a));
  for (i = 0; i < 6; i++)
    EXPECT_EQ_DOUBLE((double)i + 2,
                     lept_get_number(lept_get_array_element(&a, i)));

  lept_set_string(&e, "Hello", 5);
  lept_move(lept_pushback_array_element(&a), &e); /* Test if element is freed */
  e.reset();

  i = lept_get_array_capacity(&a);
  lept_clear_array(&a);
  EXPECT_EQ_SIZE_T(0, lept_get_array_size(&a));
  EXPECT_EQ_SIZE_T(
      i, lept_get_array_capacity(&a)); /* capacity remains unchanged */
  lept_shrink_array(&a);
  EXPECT_EQ_SIZE_T(0, lept_get_array_capacity(&a));

  a.reset();
}

static void test_access_object() {
  /* Note: Commented out in original code */
}

int main() {
  test_access_null();
  test_access_boolean();
  test_access_number();
  test_access_string();
  test_access_array();
  test_access_object();
  
  printf("%d/%d (%3.2f%%) passed\n", test_pass, test_count,
         test_pass * 100.0 / test_count);
  return main_ret;
} 