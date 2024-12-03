#include <assert.h>
#include <string.h>
#include <utf.h>

#include "../include/appling.h"

#define ID "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"

#define test_parse_hex(input, expected_id, expected_data) \
  { \
    appling_link_t link; \
    int err = appling_parse(input, &link); \
    assert(err == 0); \
    assert(strcmp(link.id, expected_id) == 0); \
    assert(strcmp(link.data, expected_data) == 0); \
  }

int
main() {
  test_parse_hex("pear://" ID, ID, "");
  test_parse_hex("punch://" ID, ID, "");

  test_parse_hex("pear://" ID "/", ID, "");
  test_parse_hex("punch://" ID "/", ID, "");

  test_parse_hex("pear://" ID "/data", ID, "data");
  test_parse_hex("punch://" ID "/data", ID, "data");

  return 0;
}
