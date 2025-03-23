#include "lept_test.h"

#define TEST_EQUAL(json1, json2, equality)                                     \
  do {                                                                         \
    lept_value v1, v2;                                                         \
    EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&v1, json1));                      \
    EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&v2, json2));                      \
    EXPECT_EQ_INT(equality, lept_is_equal(&v1, &v2));                          \
    v1.reset();                                                                \
    v2.reset();                                                                \
  } while (0)

int main() {
  TEST_EQUAL("true", "true", 1);
  TEST_EQUAL("true", "false", 0);
  TEST_EQUAL("false", "false", 1);
  TEST_EQUAL("null", "null", 1);
  TEST_EQUAL("null", "0", 0);
  TEST_EQUAL("123", "123", 1);
  TEST_EQUAL("123", "456", 0);
  TEST_EQUAL("\"abc\"", "\"abc\"", 1);
  TEST_EQUAL("\"abc\"", "\"abcd\"", 0);
  TEST_EQUAL("[]", "[]", 1);
  TEST_EQUAL("[]", "null", 0);
  TEST_EQUAL("[1,2,3]", "[1,2,3]", 1);
  TEST_EQUAL("[1,2,3]", "[1,2,3,4]", 0);
  TEST_EQUAL("[[]]", "[[]]", 1);
  TEST_EQUAL("{}", "{}", 1);
  TEST_EQUAL("{}", "null", 0);
  TEST_EQUAL("{}", "[]", 0);
  TEST_EQUAL("{\"a\":1,\"b\":2}", "{\"a\":1,\"b\":2}", 1);
  TEST_EQUAL("{\"a\":1,\"b\":2}", "{\"b\":2,\"a\":1}", 1);
  TEST_EQUAL("{\"a\":1,\"b\":2}", "{\"a\":1,\"b\":3}", 0);
  TEST_EQUAL("{\"a\":1,\"b\":2}", "{\"a\":1,\"b\":2,\"c\":3}", 0);
  TEST_EQUAL("{\"a\":{\"b\":{\"c\":{}}}}", "{\"a\":{\"b\":{\"c\":{}}}}", 1);
  TEST_EQUAL("{\"a\":{\"b\":{\"c\":{}}}}", "{\"a\":{\"b\":{\"c\":[]}}}", 0);

  printf("%d/%d (%3.2f%%) passed\n", test_pass, test_count,
         test_pass * 100.0 / test_count);
  return main_ret;
}