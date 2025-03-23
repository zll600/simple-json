#include "lept_test.h"
#include <cassert>

#define TEST_STRING(expect, json)                                              \
  do {                                                                         \
    lept_value v;                                                              \
                                                                               \
    EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&v, json));                        \
    EXPECT_EQ_INT(LEPT_STRING, lept_get_type(&v));                             \
    EXPECT_EQ_STRING(expect, lept_get_string(&v), lept_get_string_length(&v)); \
  } while (0)

static bool test_string(const std::string &expect, const std::string &json) {
  lept_value v;
  if (lept_parse(&v, json.c_str()) != LEPT_PARSE_OK) {
    return false;
  }
  if (lept_get_type(&v) != LEPT_STRING) {
    return false;
  }
  if (v.lept_get_string() != expect) {
    return false;
  }
  if (v.lept_get_string_length() != expect.length()) {
    return false;
  }
  return true;
}

int mainnew() {}

int main() {
  assert(test_string("", "\"\""));
  assert(test_string("Hello", "\"Hello\""));
  assert(test_string("Hello\nWorld", "\"Hello\\nWorld\""));
  assert(test_string("\" \\ / \b \f \n \r \t",
                     "\"\\\" \\\\ \\/ \\b \\f \\n \\r \\t\""));
  // TODO: fix this
  // assert(test_string("Hello\\0World", "\"Hello\\u0000World\""));
  assert(test_string("\x24", "\"\\u0024\""));         /* Dollar sign U+0024 */
  assert(test_string("\xC2\xA2", "\"\\u00A2\""));     /* Cents sign U+00A2 */
  assert(test_string("\xE2\x82\xAC", "\"\\u20AC\"")); /* Euro sign U+20AC */
  assert(test_string("\xF0\x9D\x84\x9E",
                     "\"\\uD834\\uDD1E\"")); /* G clef sign U+1D11E */
  assert(test_string("\xF0\x9D\x84\x9E",
                     "\"\\ud834\\udd1e\"")); /* G clef sign U+1D11E */
  return 0;
}
