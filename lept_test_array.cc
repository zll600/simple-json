#include "lept_test.h"
#include <cassert>

int mainOK() {
  lept_value v;

  assert(lept_parse(&v, "[ ]") == LEPT_PARSE_OK);
  assert(lept_get_type(&v) == LEPT_ARRAY);
  assert(v.lept_get_array_size() == 0);
  v.reset();

  return 0;
}

int main() {
  lept_value v;

  assert(lept_parse(&v, "[ null , false , true , 123 , \"abc\" ]") ==
         LEPT_PARSE_OK);
  assert(lept_get_type(&v) == LEPT_ARRAY);
  assert(v.lept_get_array_size() == 5);
  assert(lept_get_type(lept_get_array_element(&v, 0)) == LEPT_NULL);
  assert(lept_get_type(lept_get_array_element(&v, 1)) == LEPT_FALSE);
  assert(lept_get_type(lept_get_array_element(&v, 2)) == LEPT_TRUE);
  assert(lept_get_type(lept_get_array_element(&v, 3)) == LEPT_NUMBER);
  assert(lept_get_type(lept_get_array_element(&v, 4)) == LEPT_STRING);
  assert(lept_get_number(lept_get_array_element(&v, 3)) == 123.0);

  v.reset();
  assert(lept_parse(&v, "[ [ ] , [ 0 ] , [ 0 , 1 ] , [ 0 , 1 , 2 ] ]") ==
         LEPT_PARSE_OK);
  assert(lept_get_type(&v) == LEPT_ARRAY);
  assert(v.lept_get_array_size() == 4);
  for (size_t i = 0; i < 4; i++) {
    lept_value *a = lept_get_array_element(&v, i);
    assert(lept_get_type(a) == LEPT_ARRAY);
    assert(a->lept_get_array_size() == i);
    for (size_t j = 0; j < i; j++) {
      lept_value *e = lept_get_array_element(a, j);
      assert(lept_get_type(e) == LEPT_NUMBER);
      assert(lept_get_number(e) == (double)j);
    }
  }

  return 0;
}
