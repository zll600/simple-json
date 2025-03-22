#include "lept_test.h"

int main() {
  size_t i, j;
  lept_value v;

  EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&v, "[ ]"));
  EXPECT_EQ_INT(LEPT_ARRAY, lept_get_type(&v));
  EXPECT_EQ_SIZE_T(0, lept_get_array_size(&v));

  v.reset();

  EXPECT_EQ_INT(LEPT_PARSE_OK,
                lept_parse(&v, "[ null , false , true , 123 , \"abc\" ]"));
  EXPECT_EQ_INT(LEPT_ARRAY, lept_get_type(&v));
  EXPECT_EQ_SIZE_T(5, lept_get_array_size(&v));
  EXPECT_EQ_INT(LEPT_NULL, lept_get_type(lept_get_array_element(&v, 0)));
  EXPECT_EQ_INT(LEPT_FALSE, lept_get_type(lept_get_array_element(&v, 1)));
  EXPECT_EQ_INT(LEPT_TRUE, lept_get_type(lept_get_array_element(&v, 2)));
  EXPECT_EQ_INT(LEPT_NUMBER, lept_get_type(lept_get_array_element(&v, 3)));
  EXPECT_EQ_INT(LEPT_STRING, lept_get_type(lept_get_array_element(&v, 4)));
  EXPECT_EQ_DOUBLE(123.0, lept_get_number(lept_get_array_element(&v, 3)));
  EXPECT_EQ_STRING("abc", lept_get_string(lept_get_array_element(&v, 4)),
                   lept_get_string_length(lept_get_array_element(&v, 4)));
  v.reset();

  EXPECT_EQ_INT(LEPT_PARSE_OK,
                lept_parse(&v, "[ [ ] , [ 0 ] , [ 0 , 1 ] , [ 0 , 1 , 2 ] ]"));
  EXPECT_EQ_INT(LEPT_ARRAY, lept_get_type(&v));
  EXPECT_EQ_SIZE_T(4, lept_get_array_size(&v));
  for (i = 0; i < 4; i++) {
    lept_value *a = lept_get_array_element(&v, i);
    EXPECT_EQ_INT(LEPT_ARRAY, lept_get_type(a));
    EXPECT_EQ_SIZE_T(i, lept_get_array_size(a));
    for (j = 0; j < i; j++) {
      lept_value *e = lept_get_array_element(a, j);
      EXPECT_EQ_INT(LEPT_NUMBER, lept_get_type(e));
      EXPECT_EQ_DOUBLE((double)j, lept_get_number(e));
    }
  }
  v.reset();
  
  printf("%d/%d (%3.2f%%) passed\n", test_pass, test_count,
         test_pass * 100.0 / test_count);
  return main_ret;
} 