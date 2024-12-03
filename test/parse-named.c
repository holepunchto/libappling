#include <assert.h>
#include <string.h>
#include <utf.h>

#include "../include/appling.h"

#define test_parse_named(input, expected_id, expected_data) \
  { \
    appling_link_t link; \
    int err = appling_parse(input, &link); \
    assert(err == 0); \
    assert(strcmp(link.id, expected_id) == 0); \
    assert(strcmp(link.data, expected_data) == 0); \
  }

int
main() {
  test_parse_named("pear://foo", "foo", "");
  test_parse_named("punch://foo", "foo", "");

  test_parse_named("pear://foo/data", "foo", "data");
  test_parse_named("punch://foo/data", "foo", "data");

  return 0;
}
