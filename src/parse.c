#include <stdio.h>

#include "../include/appling.h"

int
appling_parse (const char *link, appling_link_t *result) {
  int res = sscanf(link, "punch://%64s/%4096s", result->key, result->data);

  if (res != 2) res = sscanf(link, "holepunch://%64s/%4096s", result->key, result->data);

  return res != 2 ? UV_EINVAL : 0;
}
