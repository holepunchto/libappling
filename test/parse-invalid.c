#include <assert.h>

#include "../include/appling.h"

#define ID "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"

#define test_parse_invalid(input) \
  { \
    appling_link_t link; \
    int err = appling_parse(input, &link); \
    assert(err < 0); \
  }

int
main() {
  // Empty
  test_parse_invalid("");

  // Empty link
  test_parse_invalid("pear://");
  test_parse_invalid("punch://");
  test_parse_invalid("pear:///");
  test_parse_invalid("punch:///");

  // Key longer than 64 characters
  test_parse_invalid("pear://" ID "aaaa");
  test_parse_invalid("punch://" ID "aaaa");

  // Incorrect protocol
  test_parse_invalid("holepunch://" ID);

  return 0;
}
