#include <assert.h> /* assert() */
#include <cctype>
#include <cstdio>  /* sprintf() */
#include <cstdlib> /* NULL, malloc(), realloc(), free(), strtod() */
#include <cstring> /* memcpy() */
#include <vector>

#include "lept_json.h"

#define EXPECT(c, ch)                                                          \
  do {                                                                         \
    assert(*c->json == (ch));                                                  \
    c->json++;                                                                 \
  } while (0)

struct lept_context {
  std::vector<char> json;
  std::vector<char>::iterator json_iter;
  std::vector<char> char_stack;
  std::vector<std::vector<char>> string_stack;
  std::vector<lept_value> lept_value_stack;
  std::vector<lept_member *> lept_member_stack;
};

void lept_context_push(lept_context *c, std::vector<char> v) {
  c->string_stack.emplace_back(v);
}

void lept_context_push(lept_context *c, char ch) {
  c->char_stack.emplace_back(ch);
}

void lept_context_push(lept_context *c, lept_value v) {
  c->lept_value_stack.emplace_back(v);
}

void lept_context_push(lept_context *c, lept_member *m) {
  c->lept_member_stack.emplace_back(m);
}

char lept_context_pop_char(lept_context *c) {
  char ret = c->char_stack.back();
  c->char_stack.pop_back();
  return ret;
}

lept_value lept_context_pop_lept_value(lept_context *c) {
  lept_value ret = c->lept_value_stack.back();
  c->lept_value_stack.pop_back();
  return ret;
}

lept_member *lept_context_pop_lept_member(lept_context *c) {
  lept_member *ret = c->lept_member_stack.back();
  c->lept_member_stack.pop_back();
  return ret;
}

std::vector<char> lept_context_pop_string(lept_context *c) {
  std::vector<char> ret = c->string_stack.back();
  c->string_stack.pop_back();
  return ret;
}

std::vector<char>::iterator
lept_parse_whitespace(lept_context *c, std::vector<char>::iterator iter) {
  while (iter != c->json.end() &&
         (*iter == ' ' || *iter == '\t' || *iter == '\n' || *iter == '\r')) {
    iter++;
  }
  return iter;
}

lept_parse_result lept_parse_literal(lept_context *c, lept_value *v,
                                     const char *literal, lept_type type) {
  assert(*c->json_iter == literal[0]);
  c->json_iter++;
  for (size_t i = 0; literal[i + 1]; i++) {
    if (*c->json_iter != literal[i + 1]) {
      return LEPT_PARSE_INVALID_VALUE;
    }
    c->json_iter++;
  }
  v->type = type;
  return LEPT_PARSE_OK;
}

lept_parse_result lept_parse_number(lept_context *c, lept_value *v) {
  std::vector<char>::iterator p = c->json_iter;
  // - sign part
  // and int part, the int maybe 0 or [1-9][0-9]*
  if (*c->json_iter == '-') {
    c->json_iter++;
  }
  if (*c->json_iter == '0') {
    c->json_iter++;
  } else {
    if (!std::isdigit(*c->json_iter)) {
      return LEPT_PARSE_INVALID_VALUE;
    }
    for (c->json_iter++; std::isdigit(*c->json_iter); c->json_iter++) {
    }
  }

  // - fractional part
  if (*c->json_iter == '.') {
    c->json_iter++;
    if (!std::isdigit(*c->json_iter)) {
      return LEPT_PARSE_INVALID_VALUE;
    }

    for (c->json_iter++; std::isdigit(*c->json_iter); c->json_iter++) {
    }
  }

  // the exponent part
  if (*c->json_iter == 'e' || *c->json_iter == 'E') {
    c->json_iter++;
    if (*c->json_iter == '+' || *c->json_iter == '-')
      c->json_iter++;
    if (!std::isdigit(*c->json_iter))
      return LEPT_PARSE_INVALID_VALUE;
    for (c->json_iter++; std::isdigit(*c->json_iter); c->json_iter++) {
    }
  }

  // convert the string to double
  v->number = std::stod(std::string(p, c->json_iter));
  if (std::isnan(v->number)) {
    return LEPT_PARSE_INVALID_VALUE;
  }
  if (std::isinf(v->number)) {
    return LEPT_PARSE_NUMBER_TOO_BIG;
  }

  v->type = LEPT_NUMBER;
  return LEPT_PARSE_OK;
}

std::vector<char>::iterator lept_parse_hex4(std::vector<char>::iterator p,
                                            unsigned *u) {
  int i;
  *u = 0;
  for (i = 0; i < 4; i++) {
    char ch = *p++;
    *u <<= 4;
    if (ch >= '0' && ch <= '9')
      *u |= ch - '0';
    else if (ch >= 'A' && ch <= 'F')
      *u |= ch - ('A' - 10);
    else if (ch >= 'a' && ch <= 'f')
      *u |= ch - ('a' - 10);
    else
      return p;
  }
  return p;
}

void lept_encode_utf8(lept_context *c, unsigned u) {
  if (u <= 0x7F)
    lept_context_push(c, u & 0xFF);
  else if (u <= 0x7FF) {
    lept_context_push(c, 0xC0 | ((u >> 6) & 0xFF));
    lept_context_push(c, 0x80 | (u & 0x3F));
  } else if (u <= 0xFFFF) {
    lept_context_push(c, 0xE0 | ((u >> 12) & 0xFF));
    lept_context_push(c, 0x80 | ((u >> 6) & 0x3F));
    lept_context_push(c, 0x80 | (u & 0x3F));
  } else {
    assert(u <= 0x10FFFF);
    lept_context_push(c, 0xF0 | ((u >> 18) & 0xFF));
    lept_context_push(c, 0x80 | ((u >> 12) & 0x3F));
    lept_context_push(c, 0x80 | ((u >> 6) & 0x3F));
    lept_context_push(c, 0x80 | (u & 0x3F));
  }
}

lept_parse_result lept_parse_string_raw(lept_context *c,
                                        std::vector<char> *str) {
  unsigned u, u2;
  assert(*c->json_iter == '\"');
  c->json_iter++;
  size_t size = 0;
  for (;;) {
    char ch = *c->json_iter++;
    switch (ch) {
    case '\"': {
      std::vector<char> s;
      for (size_t i = 0; i < size; i++) {
        s.push_back(lept_context_pop_char(c));
      }
      std::reverse(s.begin(), s.end());
      *str = s;
      return LEPT_PARSE_OK;
    }
    case '\\': {
      switch (*c->json_iter++) {
      case '\"':
        lept_context_push(c, '\"');
        break;
      case '\\':
        lept_context_push(c, '\\');
        break;
      case '/':
        lept_context_push(c, '/');
        break;
      case 'b':
        lept_context_push(c, '\b');
        break;
      case 'f':
        lept_context_push(c, '\f');
        break;
      case 'n':
        lept_context_push(c, '\n');
        break;
      case 'r':
        lept_context_push(c, '\r');
        break;
      case 't':
        lept_context_push(c, '\t');
        break;
      case 'u':
        if ((c->json_iter = lept_parse_hex4(c->json_iter, &u)) ==
            c->json.end()) {
          return LEPT_PARSE_INVALID_UNICODE_HEX;
        }

        if (u >= 0xD800 && u <= 0xDBFF) { /* surrogate pair */
          if (*c->json_iter++ != '\\')
            return LEPT_PARSE_INVALID_UNICODE_SURROGATE;
          if (*c->json_iter++ != 'u')
            return LEPT_PARSE_INVALID_UNICODE_SURROGATE;
          if ((c->json_iter = lept_parse_hex4(c->json_iter, &u2)) ==
              c->json.end()) {
            return LEPT_PARSE_INVALID_UNICODE_HEX;
          }
          if (u2 < 0xDC00 || u2 > 0xDFFF)
            return LEPT_PARSE_INVALID_UNICODE_SURROGATE;
          u = (((u - 0xD800) << 10) | (u2 - 0xDC00)) + 0x10000;
        }
        lept_encode_utf8(c, u);
        break;
      default:
        return LEPT_PARSE_INVALID_STRING_ESCAPE;
      }
      break;
    }
    case '\0': {
      return LEPT_PARSE_MISS_QUOTATION_MARK;
    }
    default: {
      if ((unsigned char)ch < 0x20)
        return LEPT_PARSE_INVALID_STRING_CHAR;
      lept_context_push(c, ch);
    }
    }
    size++;
  }
}

lept_parse_result lept_parse_string(lept_context *c, lept_value *v) {
  lept_parse_result ret = LEPT_PARSE_INVALID_VALUE;
  std::vector<char> s;
  if ((ret = lept_parse_string_raw(c, &s)) == LEPT_PARSE_OK) {
    lept_set_string(v, s);
  }
  return ret;
}

// forward declaration
lept_parse_result lept_parse_value(lept_context *c, lept_value *v);

lept_parse_result lept_parse_array(lept_context *c, lept_value *v) {
  lept_parse_result ret = LEPT_PARSE_INVALID_VALUE;
  assert(*c->json_iter == '[');
  c->json_iter++;
  c->json_iter = lept_parse_whitespace(c, c->json_iter);
  if (*c->json_iter == ']') {
    c->json_iter++;
    lept_set_array(v, 0);
    return LEPT_PARSE_OK;
  }

  size_t size = 0;
  for (;;) {
    lept_value e;
    if ((ret = lept_parse_value(c, &e)) != LEPT_PARSE_OK) {
      break;
    }
    lept_context_push(c, e);
    size++;
    c->json_iter = lept_parse_whitespace(c, c->json_iter);
    if (*c->json_iter == ',') {
      c->json_iter++;
      c->json_iter = lept_parse_whitespace(c, c->json_iter);
    } else if (*c->json_iter == ']') {
      c->json_iter++;
      lept_set_array(v, size);
      std::vector<lept_value> s;
      for (size_t i = 0; i < size; i++) {
        s.push_back(lept_context_pop_lept_value(c));
      }
      std::reverse(s.begin(), s.end());
      v->seq_array.insert(v->seq_array.end(), s.begin(), s.end());
      return LEPT_PARSE_OK;
    } else {
      ret = LEPT_PARSE_MISS_COMMA_OR_SQUARE_BRACKET;
      break;
    }
  }
  /* Pop and free values on the stack */
  for (size_t i = 0; i < size; i++) {
    lept_value e = lept_context_pop_lept_value(c);
    e.reset();
  }
  return ret;
}

lept_parse_result lept_parse_object(lept_context *c, lept_value *v) {
  lept_parse_result ret = LEPT_PARSE_INVALID_VALUE;
  assert(*c->json_iter == '{');
  c->json_iter++;
  c->json_iter = lept_parse_whitespace(c, c->json_iter);
  if (*c->json_iter == '}') {
    c->json_iter++;
    lept_set_object(v, 0);
    return LEPT_PARSE_OK;
  }
  size_t size = 0;
  lept_member m;
  for (;;) {
    std::vector<char> str;
    m.value->reset();
    /* parse key */
    if (*c->json_iter != '"') {
      ret = LEPT_PARSE_MISS_KEY;
      break;
    }
    // c->json_iter++;
    if ((ret = lept_parse_string_raw(c, &str)) != LEPT_PARSE_OK) {
      break;
    }
    m.key.assign(str.begin(), str.end());

    /* parse ws colon ws */
    c->json_iter = lept_parse_whitespace(c, c->json_iter);
    if (*c->json_iter != ':') {
      ret = LEPT_PARSE_MISS_COLON;
      break;
    }
    c->json_iter++;
    c->json_iter = lept_parse_whitespace(c, c->json_iter);

    /* parse value */
    if ((ret = lept_parse_value(c, m.value)) != LEPT_PARSE_OK)
      break;
    lept_context_push(c, &m);
    size++;
    // TODO: use std::string
    // m.k = NULL; /* ownership is transferred to member on stack */

    /* parse ws [comma | right-curly-brace] ws */
    c->json_iter = lept_parse_whitespace(c, c->json_iter);
    if (*c->json_iter == ',') {
      c->json_iter++;
      c->json_iter = lept_parse_whitespace(c, c->json_iter);
    } else if (*c->json_iter == '}') {
      c->json_iter++;
      lept_set_object(v, size);
      v->object.push_back(*lept_context_pop_lept_member(c));
      return LEPT_PARSE_OK;
    } else {
      ret = LEPT_PARSE_MISS_COMMA_OR_CURLY_BRACKET;
      break;
    }
  }
  /* Pop and free members on the stack */
  for (size_t i = 0; i < size; i++) {
    lept_member *m = lept_context_pop_lept_member(c);
    m->value->reset();
  }
  v->type = LEPT_NULL;
  return ret;
}

lept_parse_result lept_parse_value(lept_context *c, lept_value *v) {
  switch (*c->json_iter) {
  case 't':
    return lept_parse_literal(c, v, "true", LEPT_TRUE);
  case 'f':
    return lept_parse_literal(c, v, "false", LEPT_FALSE);
  case 'n':
    return lept_parse_literal(c, v, "null", LEPT_NULL);
  case '"':
    return lept_parse_string(c, v);
  case '[':
    return lept_parse_array(c, v);
  case '{':
    return lept_parse_object(c, v);
  case '\0':
    return LEPT_PARSE_EXPECT_VALUE;
  default:
    return lept_parse_number(c, v);
  }
}

lept_parse_result lept_parse(lept_value *v, const char *json) {
  assert(v != nullptr && json != nullptr);
  lept_context c;
  lept_parse_result ret = LEPT_PARSE_INVALID_VALUE;
  c.json.assign(json, json + strlen(json));
  c.json_iter = c.json.begin();

  v->reset();
  c.json_iter = lept_parse_whitespace(&c, c.json_iter);
  if ((ret = lept_parse_value(&c, v)) == LEPT_PARSE_OK) {
    c.json_iter = lept_parse_whitespace(&c, c.json_iter);
    if (c.json_iter != c.json.end()) {
      v->type = LEPT_NULL;
      ret = LEPT_PARSE_ROOT_NOT_SINGULAR;
    }
  }

  return ret;
}

// TODO: use std::string
void lept_stringify_string(lept_context *c, const char *s, size_t len) {
  static constexpr char hex_digits[] = {'0', '1', '2', '3', '4', '5', '6', '7',
                                        '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
  assert(s != nullptr);
  std::vector<char> str = std::vector<char>(len * 6 + 2);
  std::vector<char>::iterator p = str.begin();
  *p++ = '"';
  for (size_t i = 0; i < len; i++) {
    unsigned char ch = (unsigned char)s[i];
    switch (ch) {
    case '\"':
      *p++ = '\\';
      *p++ = '\"';
      break;
    case '\\':
      *p++ = '\\';
      *p++ = '\\';
      break;
    case '\b':
      *p++ = '\\';
      *p++ = 'b';
      break;
    case '\f':
      *p++ = '\\';
      *p++ = 'f';
      break;
    case '\n':
      *p++ = '\\';
      *p++ = 'n';
      break;
    case '\r':
      *p++ = '\\';
      *p++ = 'r';
      break;
    case '\t':
      *p++ = '\\';
      *p++ = 't';
      break;
    default:
      if (ch < 0x20) {
        *p++ = '\\';
        *p++ = 'u';
        *p++ = '0';
        *p++ = '0';
        *p++ = hex_digits[ch >> 4];
        *p++ = hex_digits[ch & 15];
      } else
        *p++ = s[i];
    }
  }
  *p++ = '"';
  lept_context_push(c, str);
}

static void lept_stringify_value(lept_context *c, const lept_value *v) {
  switch (v->type) {
  case LEPT_NULL:
    lept_context_push(c, std::vector<char>({'n', 'u', 'l', 'l'}));
    break;
  case LEPT_FALSE:
    lept_context_push(c, std::vector<char>({'f', 'a', 'l', 's', 'e'}));
    break;
  case LEPT_TRUE:
    lept_context_push(c, std::vector<char>({'t', 'r', 'u', 'e'}));
    break;
  case LEPT_NUMBER: {
    char buf[32];
    int len = snprintf(buf, sizeof(buf), "%.17g", v->number);
    lept_context_push(c, std::vector<char>(buf, buf + len));
    break;
  }
  case LEPT_STRING:
    lept_stringify_string(c, v->literal_string.data(),
                          v->literal_string.size());
    break;
  case LEPT_ARRAY:
    lept_context_push(c, '[');
    for (size_t i = 0; i < v->seq_array.size(); i++) {
      if (i > 0)
        lept_context_push(c, ',');
      lept_stringify_value(c, &v->seq_array[i]);
    }
    lept_context_push(c, ']');
    break;
  case LEPT_OBJECT:
    lept_context_push(c, '{');
    for (size_t i = 0; i < v->object.size(); i++) {
      if (i > 0)
        lept_context_push(c, ',');
      lept_stringify_string(c, v->object[i].key.data(),
                            v->object[i].key.size());
      lept_context_push(c, ':');
      lept_stringify_value(c, v->object[i].value);
    }
    lept_context_push(c, '}');
    break;
  default:
    assert(0 && "invalid type");
  }
}

char *lept_stringify(const lept_value *v, size_t *length) {
  lept_context c;
  assert(v != nullptr);
  lept_stringify_value(&c, v);
  if (length) {
    *length = c.char_stack.size();
  }
  lept_context_push(&c, '\0');
  return c.char_stack.data();
}

void lept_copy(lept_value *dst, const lept_value *src) {
  assert(src != nullptr && dst != nullptr && src != dst);
  switch (src->type) {
  case LEPT_STRING:
    lept_set_string(dst, src->literal_string);
    break;
  case LEPT_ARRAY:
    /* \todo */
    break;
  case LEPT_OBJECT:
    /* \todo */
    break;
  default:
    dst->reset();
    memcpy(dst, src, sizeof(lept_value));
    break;
  }
}

void lept_move(lept_value *dst, lept_value *src) {
  assert(dst != nullptr && src != nullptr && src != dst);
  dst->reset();
  memcpy(dst, src, sizeof(lept_value));
  src->reset();
}

void lept_swap(lept_value *lhs, lept_value *rhs) {
  assert(lhs != nullptr && rhs != nullptr);
  if (lhs != rhs) {
    lept_value temp;
    memcpy(&temp, lhs, sizeof(lept_value));
    memcpy(lhs, rhs, sizeof(lept_value));
    memcpy(rhs, &temp, sizeof(lept_value));
  }
}

lept_type lept_get_type(const lept_value *v) {
  assert(v != nullptr);
  return v->type;
}

int lept_is_equal(const lept_value *lhs, const lept_value *rhs) {
  assert(lhs != nullptr && rhs != nullptr);
  if (lhs->type != rhs->type)
    return 0;
  switch (lhs->type) {
  case LEPT_STRING:
    return lhs->literal_string.size() == rhs->literal_string.size() &&
           memcmp(lhs->literal_string.data(), rhs->literal_string.data(),
                  lhs->literal_string.size()) == 0;
  case LEPT_NUMBER:
    return lhs->number == rhs->number;
  case LEPT_ARRAY: {
    if (lhs->seq_array.size() != rhs->seq_array.size())
      return 0;
    for (size_t i = 0; i < lhs->seq_array.size(); i++)
      if (!lept_is_equal(&lhs->seq_array[i], &rhs->seq_array[i]))
        return 0;
    return 1;
  }
  case LEPT_OBJECT:
    /* \todo */
    return 1;
  default:
    return 1;
  }
}

int lept_get_boolean(const lept_value *v) {
  assert(v != NULL && (v->type == LEPT_TRUE || v->type == LEPT_FALSE));
  return v->type == LEPT_TRUE;
}

void lept_value::lept_set_boolean(int b) {
  reset();
  type = b ? LEPT_TRUE : LEPT_FALSE;
}

double lept_get_number(const lept_value *v) {
  assert(v != nullptr && v->type == LEPT_NUMBER);
  return v->number;
}

void lept_set_number(lept_value *v, double n) {
  v->reset();
  v->number = n;
  v->type = LEPT_NUMBER;
}

std::string lept_value::lept_get_string() const {
  assert(type == LEPT_STRING);
  return std::string(literal_string.begin(), literal_string.end());
}

size_t lept_value::lept_get_string_length() const {
  assert(type == LEPT_STRING);
  return literal_string.size();
}

// TODO: use std::string
void lept_set_string(lept_value *v, const char *s, size_t len) {
  assert(v != nullptr && (s != nullptr || len == 0));
  v->reset();
  v->literal_string = std::vector<char>(s, s + len);
  v->type = LEPT_STRING;
}

void lept_set_string(lept_value *v, const std::vector<char> &s) {
  assert(v != nullptr);
  v->reset();
  v->literal_string = s;
  v->type = LEPT_STRING;
}

void lept_set_array(lept_value *v, size_t capacity) {
  assert(v != nullptr);
  v->reset();
  v->type = LEPT_ARRAY;
  v->seq_array.reserve(capacity);
}

size_t lept_value::lept_get_array_size() const {
  assert(type == LEPT_ARRAY);
  return seq_array.size();
}

size_t lept_get_array_capacity(const lept_value *v) {
  assert(v != nullptr && v->type == LEPT_ARRAY);
  return v->seq_array.capacity();
}

void lept_reserve_array(lept_value *v, size_t capacity) {
  assert(v != nullptr && v->type == LEPT_ARRAY);
  if (v->seq_array.capacity() < capacity) {
    v->seq_array.resize(capacity);
  }
}

void lept_shrink_array(lept_value *v) {
  assert(v != nullptr && v->type == LEPT_ARRAY);
  if (v->seq_array.capacity() > v->seq_array.size()) {
    v->seq_array.resize(v->seq_array.size());
  }
}

void lept_clear_array(lept_value *v) {
  assert(v != nullptr && v->type == LEPT_ARRAY);
  lept_erase_array_element(v, 0, v->seq_array.size());
}

lept_value *lept_get_array_element(lept_value *v, size_t index) {
  assert(v != nullptr && v->type == LEPT_ARRAY);
  assert(index < v->seq_array.size());
  return &v->seq_array[index];
}

lept_value *lept_pushback_array_element(lept_value *v) {
  assert(v != nullptr && v->type == LEPT_ARRAY);
  if (v->seq_array.size() == v->seq_array.capacity())
    lept_reserve_array(
        v, v->seq_array.capacity() == 0 ? 1 : v->seq_array.capacity() * 2);
  v->seq_array.push_back(lept_value());
  return &v->seq_array.back();
}

void lept_popback_array_element(lept_value *v) {
  assert(v != nullptr && v->type == LEPT_ARRAY && v->seq_array.size() > 0);
  v->seq_array[v->seq_array.size() - 1].reset();
}

lept_value *lept_insert_array_element(lept_value *v, size_t index) {
  assert(v != nullptr && v->type == LEPT_ARRAY && index <= v->seq_array.size());
  /* \todo */
  return nullptr;
}

void lept_erase_array_element(lept_value *v, size_t index, size_t count) {
  assert(v != nullptr && v->type == LEPT_ARRAY &&
         index + count <= v->seq_array.size());
  /* \todo */
}

void lept_set_object(lept_value *v, size_t capacity) {
  assert(v != nullptr);
  v->reset();
  v->type = LEPT_OBJECT;
  v->object.resize(capacity);
}

size_t lept_get_object_size(const lept_value *v) {
  assert(v != nullptr && v->type == LEPT_OBJECT);
  return v->object.size();
}

size_t lept_get_object_capacity(const lept_value *v) {
  assert(v != nullptr && v->type == LEPT_OBJECT);
  /* \todo */
  return 0;
}

void lept_reserve_object(lept_value *v, size_t capacity) {
  assert(v != nullptr && v->type == LEPT_OBJECT);
  /* \todo */
}

void lept_shrink_object(lept_value *v) {
  assert(v != nullptr && v->type == LEPT_OBJECT);
  /* \todo */
}

void lept_clear_object(lept_value *v) {
  assert(v != nullptr && v->type == LEPT_OBJECT);
  /* \todo */
}

const char *lept_get_object_key(const lept_value *v, size_t index) {
  assert(v != nullptr && v->type == LEPT_OBJECT);
  assert(index < v->object.size());
  return v->object[index].key.data();
}

size_t lept_get_object_key_length(const lept_value *v, size_t index) {
  assert(v != nullptr && v->type == LEPT_OBJECT);
  assert(index < v->object.size());
  return v->object[index].key.size();
}

lept_value *lept_get_object_value(lept_value *v, size_t index) {
  assert(v != nullptr && v->type == LEPT_OBJECT);
  assert(index < v->object.size());
  return v->object[index].value;
}

size_t lept_find_object_index(const lept_value *v, const char *key,
                              size_t klen) {
  assert(v != nullptr && v->type == LEPT_OBJECT && key != nullptr);
  for (size_t i = 0; i < v->object.size(); i++)
    if (v->object[i].key.size() == klen &&
        memcmp(v->object[i].key.data(), key, klen) == 0)
      return i;
  return LEPT_KEY_NOT_EXIST;
}

lept_value *lept_find_object_value(lept_value *v, const char *key,
                                   size_t klen) {
  size_t index = lept_find_object_index(v, key, klen);
  return index != LEPT_KEY_NOT_EXIST ? v->object[index].value : nullptr;
}

lept_value *lept_set_object_value(lept_value *v, const char *key, size_t klen) {
  assert(v != nullptr && v->type == LEPT_OBJECT && key != nullptr);
  /* \todo */
  return nullptr;
}

void lept_remove_object_value(lept_value *v, size_t index) {
  assert(v != nullptr && v->type == LEPT_OBJECT && index < v->object.size());
  /* \todo */
}