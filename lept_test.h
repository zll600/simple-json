#pragma once

#include "lept_json.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>

static int main_ret = 0;
static int test_count = 0;
static int test_pass = 0;

#define EXPECT_EQ_BASE(equality, expect, actual, format)                       \
  do {                                                                         \
    test_count++;                                                              \
    if (equality)                                                              \
      test_pass++;                                                             \
    else {                                                                     \
      fprintf(stderr, "%s:%d: expect: " format " actual: " format "\n",        \
              __FILE__, __LINE__, expect, actual);                             \
      main_ret = 1;                                                            \
    }                                                                          \
  } while (0)

#define EXPECT_EQ_INT(expect, actual)                                          \
  EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%d")
#define EXPECT_EQ_DOUBLE(expect, actual)                                       \
  EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%.17g")
#define EXPECT_EQ_STRING(expect, actual, alength)                              \
  EXPECT_EQ_BASE(sizeof(expect) - 1 == alength &&                              \
                     memcmp(expect, actual, alength + 1) == 0,                 \
                 expect, actual, "%s")
#define EXPECT_TRUE(actual) EXPECT_EQ_BASE((actual) != 0, "true", "false", "%s")
#define EXPECT_FALSE(actual)                                                   \
  EXPECT_EQ_BASE((actual) == 0, "false", "true", "%s")

#define EXPECT_EQ_SIZE_T(expect, actual)                                       \
  EXPECT_EQ_BASE((expect) == (actual), (size_t)expect, (size_t)actual, "%zu")
