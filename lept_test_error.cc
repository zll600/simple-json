#include "lept_test.h"

#define TEST_PARSE_ERROR(error, json)                                          \
  do {                                                                         \
    lept_value v;                                                              \
                                                                               \
    v.type = LEPT_FALSE;                                                       \
    EXPECT_EQ_INT(error, lept_parse(&v, json));                                \
    EXPECT_EQ_INT(LEPT_NULL, lept_get_type(&v));                               \
    v.reset();                                                                 \
  } while (0)

static void test_parse_expect_value() {
  TEST_PARSE_ERROR(LEPT_PARSE_EXPECT_VALUE, "");
  TEST_PARSE_ERROR(LEPT_PARSE_EXPECT_VALUE, " ");
}

static void test_parse_invalid_value() {
  TEST_PARSE_ERROR(LEPT_PARSE_INVALID_VALUE, "nul");
  TEST_PARSE_ERROR(LEPT_PARSE_INVALID_VALUE, "?");

  /* invalid number */
  TEST_PARSE_ERROR(LEPT_PARSE_INVALID_VALUE, "+0");
  TEST_PARSE_ERROR(LEPT_PARSE_INVALID_VALUE, "+1");
  TEST_PARSE_ERROR(LEPT_PARSE_INVALID_VALUE,
                   ".123"); /* at least one digit before '.' */
  TEST_PARSE_ERROR(LEPT_PARSE_INVALID_VALUE,
                   "1."); /* at least one digit after '.' */
  TEST_PARSE_ERROR(LEPT_PARSE_INVALID_VALUE, "INF");
  TEST_PARSE_ERROR(LEPT_PARSE_INVALID_VALUE, "inf");
  TEST_PARSE_ERROR(LEPT_PARSE_INVALID_VALUE, "NAN");
  TEST_PARSE_ERROR(LEPT_PARSE_INVALID_VALUE, "nan");

  /* invalid value in array */
  TEST_PARSE_ERROR(LEPT_PARSE_INVALID_VALUE, "[1,]");
  TEST_PARSE_ERROR(LEPT_PARSE_INVALID_VALUE, "[\"a\", nul]");
}

static void test_parse_root_not_singular() {
  TEST_PARSE_ERROR(LEPT_PARSE_ROOT_NOT_SINGULAR, "null x");

  /* invalid number */
  TEST_PARSE_ERROR(LEPT_PARSE_ROOT_NOT_SINGULAR,
                   "0123"); /* after zero should be '.' or nothing */
  TEST_PARSE_ERROR(LEPT_PARSE_ROOT_NOT_SINGULAR, "0x0");
  TEST_PARSE_ERROR(LEPT_PARSE_ROOT_NOT_SINGULAR, "0x123");
}

static void test_parse_number_too_big() {
  TEST_PARSE_ERROR(LEPT_PARSE_NUMBER_TOO_BIG, "1e309");
  TEST_PARSE_ERROR(LEPT_PARSE_NUMBER_TOO_BIG, "-1e309");
}

static void test_parse_miss_quotation_mark() {
  TEST_PARSE_ERROR(LEPT_PARSE_MISS_QUOTATION_MARK, "\"");
  TEST_PARSE_ERROR(LEPT_PARSE_MISS_QUOTATION_MARK, "\"abc");
}

static void test_parse_invalid_string_escape() {
  TEST_PARSE_ERROR(LEPT_PARSE_INVALID_STRING_ESCAPE, "\"\\v\"");
  TEST_PARSE_ERROR(LEPT_PARSE_INVALID_STRING_ESCAPE, "\"\\'\"");
  TEST_PARSE_ERROR(LEPT_PARSE_INVALID_STRING_ESCAPE, "\"\\0\"");
  TEST_PARSE_ERROR(LEPT_PARSE_INVALID_STRING_ESCAPE, "\"\\x12\"");
}

static void test_parse_invalid_string_char() {
  TEST_PARSE_ERROR(LEPT_PARSE_INVALID_STRING_CHAR, "\"\x01\"");
  TEST_PARSE_ERROR(LEPT_PARSE_INVALID_STRING_CHAR, "\"\x1F\"");
}

static void test_parse_invalid_unicode_hex() {
  TEST_PARSE_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u\"");
  TEST_PARSE_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u0\"");
  TEST_PARSE_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u01\"");
  TEST_PARSE_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u012\"");
  TEST_PARSE_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u/000\"");
  TEST_PARSE_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\uG000\"");
  TEST_PARSE_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u0/00\"");
  TEST_PARSE_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u0G00\"");
  TEST_PARSE_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u0/00\"");
  TEST_PARSE_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u00G0\"");
  TEST_PARSE_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u000/\"");
  TEST_PARSE_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u000G\"");
  TEST_PARSE_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u 123\"");
}

static void test_parse_invalid_unicode_surrogate() {
  TEST_PARSE_ERROR(LEPT_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\"");
  TEST_PARSE_ERROR(LEPT_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uDBFF\"");
  TEST_PARSE_ERROR(LEPT_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\\\\"");
  TEST_PARSE_ERROR(LEPT_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\uDBFF\"");
  TEST_PARSE_ERROR(LEPT_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\uE000\"");
}

static void test_parse_miss_comma_or_square_bracket() {
  TEST_PARSE_ERROR(LEPT_PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[1");
  TEST_PARSE_ERROR(LEPT_PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[1}");
  TEST_PARSE_ERROR(LEPT_PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[1 2");
  TEST_PARSE_ERROR(LEPT_PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[[]");
}

static void test_parse_miss_key() {
  TEST_PARSE_ERROR(LEPT_PARSE_MISS_KEY, "{:1,");
  TEST_PARSE_ERROR(LEPT_PARSE_MISS_KEY, "{1:1,");
  TEST_PARSE_ERROR(LEPT_PARSE_MISS_KEY, "{true:1,");
  TEST_PARSE_ERROR(LEPT_PARSE_MISS_KEY, "{false:1,");
  TEST_PARSE_ERROR(LEPT_PARSE_MISS_KEY, "{null:1,");
  TEST_PARSE_ERROR(LEPT_PARSE_MISS_KEY, "{[]:1,");
  TEST_PARSE_ERROR(LEPT_PARSE_MISS_KEY, "{{}:1,");
  TEST_PARSE_ERROR(LEPT_PARSE_MISS_KEY, "{\"a\":1,");
}

static void test_parse_miss_colon() {
  TEST_PARSE_ERROR(LEPT_PARSE_MISS_COLON, "{\"a\"}");
  TEST_PARSE_ERROR(LEPT_PARSE_MISS_COLON, "{\"a\",\"b\"}");
}

static void test_parse_miss_comma_or_curly_bracket() {
  TEST_PARSE_ERROR(LEPT_PARSE_MISS_COMMA_OR_CURLY_BRACKET, "{\"a\":1");
  TEST_PARSE_ERROR(LEPT_PARSE_MISS_COMMA_OR_CURLY_BRACKET, "{\"a\":1]");
  TEST_PARSE_ERROR(LEPT_PARSE_MISS_COMMA_OR_CURLY_BRACKET, "{\"a\":1 \"b\"");
  TEST_PARSE_ERROR(LEPT_PARSE_MISS_COMMA_OR_CURLY_BRACKET, "{\"a\":{}");
}

int main() {
  test_parse_expect_value();
  test_parse_invalid_value();
  test_parse_root_not_singular();
  test_parse_number_too_big();
  test_parse_miss_quotation_mark();
  test_parse_invalid_string_escape();
  test_parse_invalid_string_char();
  test_parse_invalid_unicode_hex();
  test_parse_invalid_unicode_surrogate();
  test_parse_miss_comma_or_square_bracket();
  test_parse_miss_key();
  test_parse_miss_colon();
  test_parse_miss_comma_or_curly_bracket();

  printf("%d/%d (%3.2f%%) passed\n", test_pass, test_count,
         test_pass * 100.0 / test_count);
  return main_ret;
}