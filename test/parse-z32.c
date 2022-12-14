#include <assert.h>
#include <string.h>
#include <z32.h>

#include "../include/appling.h"

#define KEY "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"

#define test_parse_z32(input, expected_key, expected_data) \
  { \
    appling_link_t link; \
    int err = appling_parse(input, &link); \
    assert(err == 0); \
    uint8_t expected_z32_key[strlen(expected_key)]; \
    size_t expected_z32_len = strlen(expected_key); \
    z32_decode(expected_key, strlen(expected_key), expected_z32_key, &expected_z32_len); \
    assert(memcmp(link.key, expected_z32_key, APPLING_KEY_LEN) == 0); \
    assert(strcmp(link.data, expected_data) == 0); \
  }

int
main () {
  test_parse_z32("punch://" KEY, KEY, "");
  test_parse_z32("holepunch://" KEY, KEY, "");

  test_parse_z32("punch://" KEY "/", KEY, "");
  test_parse_z32("holepunch://" KEY "/", KEY, "");

  test_parse_z32("punch://" KEY "/data", KEY, "data");
  test_parse_z32("holepunch://" KEY "/data", KEY, "data");

  return 0;
}
