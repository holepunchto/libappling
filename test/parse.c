#include <assert.h>
#include <string.h>

#include "../include/appling.h"

#define KEY "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"

#define test_parse(input, expected_key, expected_data) \
  { \
    appling_link_t link; \
    int err = appling_parse(input, &link); \
    assert(err == 0); \
    assert(strcmp(link.key, expected_key) == 0); \
    assert(strcmp(link.data, expected_data) == 0); \
  }

int
main () {
  test_parse("punch://", "", "");
  test_parse("holepunch://", "", "");

  test_parse("punch:///", "", "");
  test_parse("holepunch:///", "", "");

  test_parse("punch:///data", "", "data");
  test_parse("holepunch:///data", "", "data");

  test_parse("punch://" KEY, KEY, "");
  test_parse("holepunch://" KEY, KEY, "");
  test_parse("punch://aaaa", "aaaa", "");
  test_parse("holepunch://aaaa", "aaaa", "");

  test_parse("punch://" KEY "/", KEY, "");
  test_parse("holepunch://" KEY "/", KEY, "");
  test_parse("punch://aaaa/", "aaaa", "");
  test_parse("holepunch://aaaa/", "aaaa", "");

  test_parse("punch://" KEY "/data", KEY, "data");
  test_parse("holepunch://" KEY "/data", KEY, "data");
  test_parse("punch://aaaa/data", "aaaa", "data");
  test_parse("holepunch://aaaa/data", "aaaa", "data");

  return 0;
}
