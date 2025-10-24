#include <assert.h>
#include <string.h>
#include <utf.h>

#include "../include/appling.h"

#define ID "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"

#define test_parse_z32(input, expected_id, expected_data) \
  { \
    appling_link_t link; \
    err = appling_parse(input, &link); \
    assert(err == 0); \
    assert(strcmp(link.id, expected_id) == 0); \
    assert(strcmp(link.data, expected_data) == 0); \
  }

int
main() {
  int err;

  test_parse_z32("pear://" ID, ID, "");
  test_parse_z32("punch://" ID, ID, "");

  test_parse_z32("pear://" ID "/", ID, "");
  test_parse_z32("punch://" ID "/", ID, "");

  test_parse_z32("pear://" ID "/data", ID, "data");
  test_parse_z32("punch://" ID "/data", ID, "data");

  return 0;
}
