#include <assert.h> /* assert() */
#include <cctype>
#include <cstdio>  /* sprintf() */
#include <cstdlib> /* NULL, malloc(), realloc(), free(), strtod() */
#include <cstring> /* memcpy() */
#include <vector>

#include "leptjson.h"

#define EXPECT(c, ch)                                                          \
  do {                                                                         \
    assert(*c->json == (ch));                                                  \
    c->json++;                                                                 \
  } while (0)
#define ISDIGIT(ch) ((ch) >= '0' && (ch) <= '9')
#define ISDIGIT1TO9(ch) ((ch) >= '1' && (ch) <= '9')
#define PUTC(c, ch)                                                            \
  do {                                                                         \
    *(char *)lept_context_push(c, sizeof(char)) = (ch);                        \
  } while (0)
#define PUTS(c, s, len) memcpy(lept_context_push(c, len), s, len)

// constants
static constexpr size_t LEPT_PARSE_STRINGIFY_INIT_SIZE = 256;
static constexpr size_t LEPT_PARSE_STACK_INIT_SIZE = 256;

struct lept_context {
  std::vector<char> json;
  std::vector<char> stack;
  size_t size;
  size_t top;
};

void *lept_context_push(lept_context *c, size_t size) {
  void *ret;
  assert(size > 0);
  if (c->top + size >= c->size) {
    if (c->size == 0) {
      c->size = LEPT_PARSE_STACK_INIT_SIZE;
    }

    while (c->top + size >= c->size) {
      c->size *= 2; /* c->size * 2 */
    }
    c->stack.resize(c->size);
  }
  ret = c->stack.data() + c->top;
  c->top += size;
  return ret;
}

void *lept_context_pop(lept_context *c, size_t size) {
  assert(c->top >= size);
  c->top -= size;
  return c->stack.data() + c->top;
}

void lept_parse_whitespace(lept_context *c) {
  std::vector<char>::iterator p = c->json.begin();
  while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r') {
    p++;
  }
  c->json.erase(c->json.begin(), p);
}

lept_parse_result lept_parse_literal(lept_context *c, lept_value *v,
                                     const char *literal, lept_type type) {
  assert(c->json.front() == literal[0]);
  std::vector<char>::iterator p = c->json.begin();
  p++;
  for (size_t i = 0; literal[i + 1]; i++) {
    if (*p != literal[i + 1]) {
      return LEPT_PARSE_INVALID_VALUE;
    }
    p++;
  }
  c->json.erase(c->json.begin(), p);
  v->type = type;
  return LEPT_PARSE_OK;
}

lept_parse_result lept_parse_number(lept_context *c, lept_value *v) {
  std::vector<char>::iterator p = c->json.begin();
  // - sign part
  // and int part, the int maybe 0 or [1-9][0-9]*
  if (*p == '-') {
    p++;
  } else if (*p == '0') {
    p++;
  } else {
    if (!std::isdigit(*p)) {
      return LEPT_PARSE_INVALID_VALUE;
    }
    for (p++; std::isdigit(*p); p++) {
    }
  }

  // - fractional part
  if (*p == '.') {
    p++;
    if (!std::isdigit(*p)) {
      return LEPT_PARSE_INVALID_VALUE;
    }

    for (p++; std::isdigit(*p); p++) {
    }
  }

  // the exponent part
  if (*p == 'e' || *p == 'E') {
    p++;
    if (*p == '+' || *p == '-')
      p++;
    if (!std::isdigit(*p))
      return LEPT_PARSE_INVALID_VALUE;
    for (p++; std::isdigit(*p); p++) {
    }
  }

  // convert the string to double
  v->number = std::stod(c->json.data());
  if (std::isnan(v->number)) {
    return LEPT_PARSE_INVALID_VALUE;
  }
  if (std::isinf(v->number)) {
    return LEPT_PARSE_NUMBER_TOO_BIG;
  }

  v->type = LEPT_NUMBER;
  c->json.erase(c->json.begin(), p);
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
    PUTC(c, u & 0xFF);
  else if (u <= 0x7FF) {
    PUTC(c, 0xC0 | ((u >> 6) & 0xFF));
    PUTC(c, 0x80 | (u & 0x3F));
  } else if (u <= 0xFFFF) {
    PUTC(c, 0xE0 | ((u >> 12) & 0xFF));
    PUTC(c, 0x80 | ((u >> 6) & 0x3F));
    PUTC(c, 0x80 | (u & 0x3F));
  } else {
    assert(u <= 0x10FFFF);
    PUTC(c, 0xF0 | ((u >> 18) & 0xFF));
    PUTC(c, 0x80 | ((u >> 12) & 0x3F));
    PUTC(c, 0x80 | ((u >> 6) & 0x3F));
    PUTC(c, 0x80 | (u & 0x3F));
  }
}

#define STRING_ERROR(ret)                                                      \
  do {                                                                         \
    c->top = head;                                                             \
    return ret;                                                                \
  } while (0)

lept_parse_result lept_parse_string_raw(lept_context *c,
                                        std::vector<char> *str) {
  size_t head = c->top;
  unsigned u, u2;
  assert(c->json.front() == '\"');
  std::vector<char>::iterator p = c->json.begin();
  p++;
  for (;;) {
    char ch = *p++;
    switch (ch) {
    case '\"': {
      char *tmp = (char *)lept_context_pop(c, c->top - head);
      *str = std::vector<char>(tmp, tmp + c->top - head);
      c->json.erase(c->json.begin(), p);
      return LEPT_PARSE_OK;
    }
    case '\\':
      switch (*p++) {
      case '\"':
        PUTC(c, '\"');
        break;
      case '\\':
        PUTC(c, '\\');
        break;
      case '/':
        PUTC(c, '/');
        break;
      case 'b':
        PUTC(c, '\b');
        break;
      case 'f':
        PUTC(c, '\f');
        break;
      case 'n':
        PUTC(c, '\n');
        break;
      case 'r':
        PUTC(c, '\r');
        break;
      case 't':
        PUTC(c, '\t');
        break;
      case 'u':
        if ((p = lept_parse_hex4(p, &u)) == c->json.end()) {
          STRING_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX);
        }

        if (u >= 0xD800 && u <= 0xDBFF) { /* surrogate pair */
          if (*p++ != '\\')
            STRING_ERROR(LEPT_PARSE_INVALID_UNICODE_SURROGATE);
          if (*p++ != 'u')
            STRING_ERROR(LEPT_PARSE_INVALID_UNICODE_SURROGATE);
          if ((p = lept_parse_hex4(p, &u2)) == c->json.end()) {
            STRING_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX);
          }
          if (u2 < 0xDC00 || u2 > 0xDFFF)
            STRING_ERROR(LEPT_PARSE_INVALID_UNICODE_SURROGATE);
          u = (((u - 0xD800) << 10) | (u2 - 0xDC00)) + 0x10000;
        }
        lept_encode_utf8(c, u);
        break;
      default:
        STRING_ERROR(LEPT_PARSE_INVALID_STRING_ESCAPE);
      }
      break;
    case '\0':
      STRING_ERROR(LEPT_PARSE_MISS_QUOTATION_MARK);
    default:
      if ((unsigned char)ch < 0x20)
        STRING_ERROR(LEPT_PARSE_INVALID_STRING_CHAR);
      PUTC(c, ch);
    }
  }
}

lept_parse_result lept_parse_string(lept_context *c, lept_value *v) {
  lept_parse_result ret = LEPT_PARSE_INVALID_VALUE;
  std::vector<char> s;
  if ((ret = lept_parse_string_raw(c, &s)) == LEPT_PARSE_OK) {
    lept_set_string(v, s.data(), s.size());
  }
  return ret;
}

// forward declaration
lept_parse_result lept_parse_value(lept_context *c, lept_value *v);

lept_parse_result lept_parse_array(lept_context *c, lept_value *v) {
  size_t i, size = 0;
  lept_parse_result ret = LEPT_PARSE_INVALID_VALUE;
  assert(c->json.front() == '[');
  std::vector<char>::iterator p = c->json.begin();
  p++;
  lept_parse_whitespace(c);
  if (*p == ']') {
    c->json.erase(c->json.begin(), p);
    lept_set_array(v, 0);
    return LEPT_PARSE_OK;
  }

  for (;;) {
    lept_value e;
    if ((ret = lept_parse_value(c, &e)) != LEPT_PARSE_OK)
      break;
    memcpy(lept_context_push(c, sizeof(lept_value)), &e, sizeof(lept_value));
    size++;
    lept_parse_whitespace(c);
    if (*p == ',') {
      p++;
      lept_parse_whitespace(c);
    } else if (*p == ']') {
      c->json.erase(c->json.begin(), p);
      lept_set_array(v, size);
      memcpy(v->seq_array.data(),
             lept_context_pop(c, size * sizeof(lept_value)),
             size * sizeof(lept_value));
      return LEPT_PARSE_OK;
    } else {
      ret = LEPT_PARSE_MISS_COMMA_OR_SQUARE_BRACKET;
      break;
    }
  }
  /* Pop and free values on the stack */
  for (i = 0; i < size; i++) {
    lept_value *e = (lept_value *)lept_context_pop(c, sizeof(lept_value));
    e->reset();
  }
  return ret;
}

lept_parse_result lept_parse_object(lept_context *c, lept_value *v) {
  lept_parse_result ret = LEPT_PARSE_INVALID_VALUE;
  assert(c->json.front() == '{');
  std::vector<char>::iterator p = c->json.begin();
  p++;
  lept_parse_whitespace(c);
  if (*p == '}') {
    c->json.erase(c->json.begin(), p);
    lept_set_object(v, 0);
    return LEPT_PARSE_OK;
  }
  size_t size = 0;
  lept_member m;
  for (;;) {
    std::vector<char> str;
    m.value->reset();
    /* parse key */
    if (*p != '"') {
      ret = LEPT_PARSE_MISS_KEY;
      break;
    }
    if ((ret = lept_parse_string_raw(c, &str)) != LEPT_PARSE_OK) {
      break;
    }
    m.key.assign(str.begin(), str.end());

    /* parse ws colon ws */
    lept_parse_whitespace(c);
    if (*p != ':') {
      ret = LEPT_PARSE_MISS_COLON;
      break;
    }
    p++;
    lept_parse_whitespace(c);

    /* parse value */
    if ((ret = lept_parse_value(c, m.value)) != LEPT_PARSE_OK)
      break;
    memcpy(lept_context_push(c, sizeof(lept_member)), &m, sizeof(lept_member));
    size++;
    // TODO: use std::string
    // m.k = NULL; /* ownership is transferred to member on stack */

    /* parse ws [comma | right-curly-brace] ws */
    lept_parse_whitespace(c);
    if (*p == ',') {
      p++;
      lept_parse_whitespace(c);
    } else if (*p == '}') {
      c->json.erase(c->json.begin(), p);
      lept_set_object(v, size);
      memcpy(v->object.data(), lept_context_pop(c, sizeof(lept_member) * size),
             sizeof(lept_member) * size);
      return LEPT_PARSE_OK;
    } else {
      ret = LEPT_PARSE_MISS_COMMA_OR_CURLY_BRACKET;
      break;
    }
  }
  /* Pop and free members on the stack */
  for (size_t i = 0; i < size; i++) {
    lept_member *m = (lept_member *)lept_context_pop(c, sizeof(lept_member));
    m->value->reset();
  }
  v->type = LEPT_NULL;
  return ret;
}

lept_parse_result lept_parse_value(lept_context *c, lept_value *v) {
  switch (c->json.front()) {
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
  c.stack.clear();
  c.size = c.top = 0;

  v->reset();
  lept_parse_whitespace(&c);
  if ((ret = lept_parse_value(&c, v)) == LEPT_PARSE_OK) {
    lept_parse_whitespace(&c);
    if (!c.json.empty()) {
      v->type = LEPT_NULL;
      ret = LEPT_PARSE_ROOT_NOT_SINGULAR;
    }
  }
  assert(c.top == 0);

  return ret;
}

// TODO: use std::string
void lept_stringify_string(lept_context *c, const char *s, size_t len) {
  static constexpr char hex_digits[] = {'0', '1', '2', '3', '4', '5', '6', '7',
                                        '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
  size_t i, size;
  char *head, *p;
  assert(s != nullptr);
  p = head = (char *)lept_context_push(c, size = len * 6 + 2); /* "\u00xx..." */
  *p++ = '"';
  for (i = 0; i < len; i++) {
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
  c->top -= size - (p - head);
}

static void lept_stringify_value(lept_context *c, const lept_value *v) {
  switch (v->type) {
  case LEPT_NULL:
    PUTS(c, "null", 4);
    break;
  case LEPT_FALSE:
    PUTS(c, "false", 5);
    break;
  case LEPT_TRUE:
    PUTS(c, "true", 4);
    break;
  case LEPT_NUMBER:
    c->top -=
        32 - snprintf((char *)lept_context_push(c, 32), 32, "%.17g", v->number);
    break;
  case LEPT_STRING:
    lept_stringify_string(c, v->literal_string.data(),
                          v->literal_string.size());
    break;
  case LEPT_ARRAY:
    PUTC(c, '[');
    for (size_t i = 0; i < v->seq_array.size(); i++) {
      if (i > 0)
        PUTC(c, ',');
      lept_stringify_value(c, v->seq_array[i]);
    }
    PUTC(c, ']');
    break;
  case LEPT_OBJECT:
    PUTC(c, '{');
    for (size_t i = 0; i < v->object.size(); i++) {
      if (i > 0)
        PUTC(c, ',');
      lept_stringify_string(c, v->object[i].key.data(),
                            v->object[i].key.size());
      PUTC(c, ':');
      lept_stringify_value(c, v->object[i].value);
    }
    PUTC(c, '}');
    break;
  default:
    assert(0 && "invalid type");
  }
}

char *lept_stringify(const lept_value *v, size_t *length) {
  lept_context c;
  assert(v != nullptr);
  c.stack.resize(LEPT_PARSE_STRINGIFY_INIT_SIZE);
  c.top = 0;
  lept_stringify_value(&c, v);
  if (length)
    *length = c.top;
  PUTC(&c, '\0');
  return c.stack.data();
}

void lept_copy(lept_value *dst, const lept_value *src) {
  assert(src != nullptr && dst != nullptr && src != dst);
  switch (src->type) {
  case LEPT_STRING:
    lept_set_string(dst, src->literal_string.data(),
                    src->literal_string.size());
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
      if (!lept_is_equal(lhs->seq_array[i], rhs->seq_array[i]))
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

const char *lept_get_string(const lept_value *v) {
  assert(v != nullptr && v->type == LEPT_STRING);
  return v->literal_string.data();
}

size_t lept_get_string_length(const lept_value *v) {
  assert(v != nullptr && v->type == LEPT_STRING);
  return v->literal_string.size();
}

// TODO: use std::string
void lept_set_string(lept_value *v, const char *s, size_t len) {
  assert(v != nullptr && (s != nullptr || len == 0));
  v->reset();
  v->literal_string.assign(s, s + len);
  v->type = LEPT_STRING;
}

void lept_set_array(lept_value *v, size_t capacity) {
  assert(v != nullptr);
  v->reset();
  v->type = LEPT_ARRAY;
  v->seq_array.resize(capacity);
}

size_t lept_get_array_size(const lept_value *v) {
  assert(v != nullptr && v->type == LEPT_ARRAY);
  return v->seq_array.size();
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
  return v->seq_array[index];
}

lept_value *lept_pushback_array_element(lept_value *v) {
  assert(v != nullptr && v->type == LEPT_ARRAY);
  if (v->seq_array.size() == v->seq_array.capacity())
    lept_reserve_array(
        v, v->seq_array.capacity() == 0 ? 1 : v->seq_array.capacity() * 2);
  v->seq_array.push_back(new lept_value());
  return v->seq_array.back();
}

void lept_popback_array_element(lept_value *v) {
  assert(v != nullptr && v->type == LEPT_ARRAY && v->seq_array.size() > 0);
  v->seq_array[v->seq_array.size() - 1]->reset();
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