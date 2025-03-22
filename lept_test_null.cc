#include "lept_test.h"

// test parse null
int main() {
  lept_value v;
  v.lept_set_boolean(0);
  EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&v, "null"));
  EXPECT_EQ_INT(LEPT_NULL, lept_get_type(&v));
  v.reset();
}