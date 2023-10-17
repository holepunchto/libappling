#include <assert.h>
#include <hex.h>
#include <string.h>
#include <utf.h>

#include "../include/appling.h"

#define KEY "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"

#define test_parse_hex(input, expected_key, expected_data) \
  { \
    appling_link_t link; \
    int err = appling_parse(input, &link); \
    assert(err == 0); \
    assert(strcmp(link.key, expected_key) == 0); \
    assert(strcmp(link.data, expected_data) == 0); \
  }

int
main () {
  test_parse_hex("pear://" KEY, KEY, "");
  test_parse_hex("punch://" KEY, KEY, "");

  test_parse_hex("pear://" KEY "/", KEY, "");
  test_parse_hex("punch://" KEY "/", KEY, "");

  test_parse_hex("pear://" KEY "/data", KEY, "data");
  test_parse_hex("punch://" KEY "/data", KEY, "data");

  return 0;
}
