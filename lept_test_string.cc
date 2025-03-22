#include "lept_test.h"

#define TEST_STRING(expect, json)                                              \
  do {                                                                         \
    lept_value v;                                                              \
                                                                               \
    EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&v, json));                        \
    EXPECT_EQ_INT(LEPT_STRING, lept_get_type(&v));                             \
    EXPECT_EQ_STRING(expect, lept_get_string(&v), lept_get_string_length(&v)); \
    v.reset();                                                                 \
  } while (0)

int main() {
  TEST_STRING("", "\"\"");
  TEST_STRING("Hello", "\"Hello\"");
  TEST_STRING("Hello\nWorld", "\"Hello\\nWorld\"");
  TEST_STRING("\" \\ / \b \f \n \r \t",
              "\"\\\" \\\\ \\/ \\b \\f \\n \\r \\t\"");
  TEST_STRING("Hello\0World", "\"Hello\\u0000World\"");
  TEST_STRING("\x24", "\"\\u0024\"");         /* Dollar sign U+0024 */
  TEST_STRING("\xC2\xA2", "\"\\u00A2\"");     /* Cents sign U+00A2 */
  TEST_STRING("\xE2\x82\xAC", "\"\\u20AC\""); /* Euro sign U+20AC */
  TEST_STRING("\xF0\x9D\x84\x9E",
              "\"\\uD834\\uDD1E\""); /* G clef sign U+1D11E */
  TEST_STRING("\xF0\x9D\x84\x9E",
              "\"\\ud834\\udd1e\""); /* G clef sign U+1D11E */
              
  printf("%d/%d (%3.2f%%) passed\n", test_pass, test_count,
         test_pass * 100.0 / test_count);
  return main_ret;
} 