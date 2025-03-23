#include "lept_test.h"

// test parse false
int main() {
  lept_value v;
  EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&v, "false"));
  EXPECT_EQ_INT(LEPT_FALSE, lept_get_type(&v));
  return 0;
}