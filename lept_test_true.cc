#include "lept_test.h"

// test parse true
int main() {
  lept_value v;
  EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&v, "true"));
  EXPECT_EQ_INT(LEPT_TRUE, lept_get_type(&v));
  return 0;
}