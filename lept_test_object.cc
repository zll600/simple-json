#include "lept_test.h"

int main() {
  lept_value v;
  size_t i;

  EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&v, " { } "));
  EXPECT_EQ_INT(LEPT_OBJECT, lept_get_type(&v));
  EXPECT_EQ_SIZE_T(0, lept_get_object_size(&v));
  v.reset();

  EXPECT_EQ_INT(LEPT_PARSE_OK,
                lept_parse(&v, " { "
                               "\"n\" : null , "
                               "\"f\" : false , "
                               "\"t\" : true , "
                               "\"i\" : 123 , "
                               "\"s\" : \"abc\", "
                               "\"a\" : [ 1, 2, 3 ],"
                               "\"o\" : { \"1\" : 1, \"2\" : 2, \"3\" : 3 }"
                               " } "));
  EXPECT_EQ_INT(LEPT_OBJECT, lept_get_type(&v));
  EXPECT_EQ_SIZE_T(7, lept_get_object_size(&v));
  EXPECT_EQ_STRING("n", lept_get_object_key(&v, 0),
                   lept_get_object_key_length(&v, 0));
  EXPECT_EQ_INT(LEPT_NULL, lept_get_type(lept_get_object_value(&v, 0)));
  EXPECT_EQ_STRING("f", lept_get_object_key(&v, 1),
                   lept_get_object_key_length(&v, 1));
  EXPECT_EQ_INT(LEPT_FALSE, lept_get_type(lept_get_object_value(&v, 1)));
  EXPECT_EQ_STRING("t", lept_get_object_key(&v, 2),
                   lept_get_object_key_length(&v, 2));
  EXPECT_EQ_INT(LEPT_TRUE, lept_get_type(lept_get_object_value(&v, 2)));
  EXPECT_EQ_STRING("i", lept_get_object_key(&v, 3),
                   lept_get_object_key_length(&v, 3));
  EXPECT_EQ_INT(LEPT_NUMBER, lept_get_type(lept_get_object_value(&v, 3)));
  EXPECT_EQ_DOUBLE(123.0, lept_get_number(lept_get_object_value(&v, 3)));
  EXPECT_EQ_STRING("s", lept_get_object_key(&v, 4),
                   lept_get_object_key_length(&v, 4));
  EXPECT_EQ_INT(LEPT_STRING, lept_get_type(lept_get_object_value(&v, 4)));
  EXPECT_EQ_STRING("abc", lept_get_string(lept_get_object_value(&v, 4)),
                   lept_get_string_length(lept_get_object_value(&v, 4)));
  EXPECT_EQ_STRING("a", lept_get_object_key(&v, 5),
                   lept_get_object_key_length(&v, 5));
  EXPECT_EQ_INT(LEPT_ARRAY, lept_get_type(lept_get_object_value(&v, 5)));
  EXPECT_EQ_SIZE_T(3, lept_get_array_size(lept_get_object_value(&v, 5)));
  for (i = 0; i < 3; i++) {
    lept_value *e = lept_get_array_element(lept_get_object_value(&v, 5), i);
    EXPECT_EQ_INT(LEPT_NUMBER, lept_get_type(e));
    EXPECT_EQ_DOUBLE(i + 1.0, lept_get_number(e));
  }
  EXPECT_EQ_STRING("o", lept_get_object_key(&v, 6),
                   lept_get_object_key_length(&v, 6));
  {
    lept_value *o = lept_get_object_value(&v, 6);
    EXPECT_EQ_INT(LEPT_OBJECT, lept_get_type(o));
    for (i = 0; i < 3; i++) {
      lept_value *ov = lept_get_object_value(o, i);
      EXPECT_TRUE('1' + i == lept_get_object_key(o, i)[0]);
      EXPECT_EQ_SIZE_T(1, lept_get_object_key_length(o, i));
      EXPECT_EQ_INT(LEPT_NUMBER, lept_get_type(ov));
      EXPECT_EQ_DOUBLE(i + 1.0, lept_get_number(ov));
    }
  }
  v.reset();
  
  printf("%d/%d (%3.2f%%) passed\n", test_pass, test_count,
         test_pass * 100.0 / test_count);
  return main_ret;
} 