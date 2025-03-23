#include "lept_test.h"
#include <cassert>

int main() {
  lept_value v;
  assert(lept_parse(&v, " { } ") == LEPT_PARSE_OK);
  assert(lept_get_type(&v) == LEPT_OBJECT);
  assert(lept_get_object_size(&v) == 0);
  v.reset();

  assert(lept_parse(&v, " { \"n\" : null , \"f\" : false , \"t\" : true , "
                        "\"i\" : 123 , \"s\" : \"abc\", \"a\" : [ 1, 2, 3 ], "
                        "\"o\" : { \"1\" : 1, \"2\" : 2, \"3\" : 3 }  } ") ==
         LEPT_PARSE_OK);
}

int mainOld() {
  lept_value v;
  size_t i;

  assert(lept_get_type(&v) == LEPT_OBJECT);
  assert(lept_get_object_size(&v) == 7);
  assert(std::string(lept_get_object_key(&v, 0)) == "n");
  assert(lept_get_type(lept_get_object_value(&v, 0)) == LEPT_NULL);
  assert(std::string(lept_get_object_key(&v, 1)) == "f");
  assert(lept_get_type(lept_get_object_value(&v, 1)) == LEPT_FALSE);
  assert(std::string(lept_get_object_key(&v, 2)) == "t");
  assert(lept_get_type(lept_get_object_value(&v, 2)) == LEPT_TRUE);
  assert(std::string(lept_get_object_key(&v, 3)) == "i");
  assert(lept_get_type(lept_get_object_value(&v, 3)) == LEPT_NUMBER);
  assert(lept_get_number(lept_get_object_value(&v, 3)) == 123.0);
  assert(std::string(lept_get_object_key(&v, 4)) == "s");
  assert(lept_get_type(lept_get_object_value(&v, 4)) == LEPT_STRING);
  // assert(lept_get_string(lept_get_object_value(&v, 4)) == "abc");
  // assert(lept_get_string_length(lept_get_object_value(&v, 4)) == 3);
  assert(std::string(lept_get_object_key(&v, 5)) == "a");
  assert(lept_get_type(lept_get_object_value(&v, 5)) == LEPT_ARRAY);
  // assert(lept_get_array_size(lept_get_object_value(&v, 5)) == 3);
  for (i = 0; i < 3; i++) {
    lept_value *e = lept_get_array_element(lept_get_object_value(&v, 5), i);
    assert(lept_get_type(e) == LEPT_NUMBER);
    assert(lept_get_number(e) == i + 1.0);
  }
  assert(std::string(lept_get_object_key(&v, 6)) == "o");
  {
    lept_value *o = lept_get_object_value(&v, 6);
    assert(lept_get_type(o) == LEPT_OBJECT);
    for (i = 0; i < 3; i++) {
      lept_value *ov = lept_get_object_value(o, i);
      //   assert('1' + i == lept_get_object_key(o, i)[0]);
      assert(lept_get_object_key_length(o, i) == 1);
      assert(lept_get_type(ov) == LEPT_NUMBER);
      assert(lept_get_number(ov) == i + 1.0);
    }
  }

  return 0;
}