#include "lept_test.h"

// test parse null
int main() {
  lept_value v;
  EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&v, "null"));
  EXPECT_EQ_INT(LEPT_NULL, lept_get_type(&v));
  return 0;
}