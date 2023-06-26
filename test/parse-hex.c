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
    uint8_t expected_hex_key[strlen(expected_key) + 1]; \
    size_t expected_hex_len = strlen(expected_key) + 1; \
    hex_decode((utf8_t *) expected_key, strlen(expected_key), expected_hex_key, &expected_hex_len); \
    assert(memcmp(link.key, expected_hex_key, APPLING_KEY_LEN) == 0); \
    assert(strcmp(link.data, expected_data) == 0); \
  }

int
main () {
  test_parse_hex("punch://" KEY, KEY, "");
  test_parse_hex("holepunch://" KEY, KEY, "");

  test_parse_hex("punch://" KEY "/", KEY, "");
  test_parse_hex("holepunch://" KEY "/", KEY, "");

  test_parse_hex("punch://" KEY "/data", KEY, "data");
  test_parse_hex("holepunch://" KEY "/data", KEY, "data");

  return 0;
}
