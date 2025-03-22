#include "lept_test.h"

static void test_copy() {
  lept_value v1, v2;
  lept_parse(&v1,
             "{\"t\":true,\"f\":false,\"n\":null,\"d\":1.5,\"a\":[1,2,3]}");
  lept_copy(&v2, &v1);
  EXPECT_TRUE(lept_is_equal(&v2, &v1));
  v1.reset();
  v2.reset();
}

static void test_move() {
  lept_value v1, v2, v3;
  lept_parse(&v1,
             "{\"t\":true,\"f\":false,\"n\":null,\"d\":1.5,\"a\":[1,2,3]}");
  lept_copy(&v2, &v1);
  lept_move(&v3, &v2);
  EXPECT_EQ_INT(LEPT_NULL, lept_get_type(&v2));
  EXPECT_TRUE(lept_is_equal(&v3, &v1));
  v1.reset();
  v2.reset();
  v3.reset();
}

static void test_swap() {
  lept_value v1, v2;
  lept_set_string(&v1, "Hello", 5);
  lept_set_string(&v2, "World!", 6);
  lept_swap(&v1, &v2);
  EXPECT_EQ_STRING("World!", lept_get_string(&v1), lept_get_string_length(&v1));
  EXPECT_EQ_STRING("Hello", lept_get_string(&v2), lept_get_string_length(&v2));
  v1.reset();
  v2.reset();
}

int main() {
  test_copy();
  test_move();
  test_swap();
  
  printf("%d/%d (%3.2f%%) passed\n", test_pass, test_count,
         test_pass * 100.0 / test_count);
  return main_ret;
} 