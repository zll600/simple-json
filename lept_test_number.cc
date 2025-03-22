#include "lept_test.h"
#include <cassert>
#define TEST_NUMBER(expect, json)                                              \
  do {                                                                         \
    lept_value v;                                                              \
                                                                               \
    EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&v, json));                        \
    EXPECT_EQ_INT(LEPT_NUMBER, lept_get_type(&v));                             \
    EXPECT_EQ_DOUBLE(expect, lept_get_number(&v));                             \
  } while (0)

static bool test_number(double expect, const char *json) {
  lept_value v;
  if (LEPT_PARSE_OK != lept_parse(&v, json)) {
    return false;
  }
  if (LEPT_NUMBER != lept_get_type(&v)) {
    return false;
  }
  if (expect != lept_get_number(&v)) {
    return false;
  }
  return true;
}

int main() {
  assert(test_number(0.0, "0"));
  assert(test_number(0.0, "-0"));
  assert(test_number(0.0, "-0.0"));
  assert(test_number(1.0, "1"));
  assert(test_number(-1.0, "-1"));
  assert(test_number(1.5, "1.5"));
  assert(test_number(-1.5, "-1.5"));
  assert(test_number(3.1416, "3.1416"));
  assert(test_number(1E10, "1E10"));
  assert(test_number(1e10, "1e10"));
  assert(test_number(1E+10, "1E+10"));
  assert(test_number(1E-10, "1E-10"));
  assert(test_number(-1E10, "-1E10"));
  assert(test_number(-1e10, "-1e10"));
  assert(test_number(-1E+10, "-1E+10"));
  assert(test_number(-1E-10, "-1E-10"));
  assert(test_number(1.234E+10, "1.234E+10"));
  assert(test_number(1.234E-10, "1.234E-10"));

  // assert(test_number(0.0, "1e-10000")); /* must underflow */

  assert(test_number(1.0000000000000002,
              "1.0000000000000002")); /* the smallest number > 1 */
  // TODO: fix these tests about float numbers
  // assert(test_number(4.9406564584124654e-324,
  //             "4.9406564584124654e-324")); /* minimum denormal */
  // assert(test_number(-4.9406564584124654e-324, "-4.9406564584124654e-324"));
  // assert(test_number(2.2250738585072009e-308,
  //             "2.2250738585072009e-308")); /* Max subnormal double */
  // assert(test_number(-2.2250738585072009e-308, "-2.2250738585072009e-308"));
  assert(test_number(2.2250738585072014e-308, "2.2250738585072014e-308")); /* Min normal positive double */
  assert(test_number(-2.2250738585072014e-308, "-2.2250738585072014e-308"));
  assert(test_number(1.7976931348623157e+308, "1.7976931348623157e+308")); /* Max double */
  assert(test_number(-1.7976931348623157e+308, "-1.7976931348623157e+308"));
  
  printf("%d/%d (%3.2f%%) passed\n", test_pass, test_count,
         test_pass * 100.0 / test_count);
  return main_ret;
}