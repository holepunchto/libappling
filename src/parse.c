#include <stddef.h>
#include <string.h>

#include "../include/appling.h"

int
appling_parse (const char *link, appling_link_t *result) {
  size_t link_len = strlen(link);

  if (link_len == 0) goto err;

  size_t i = 0;

  if (strncmp(link, "punch://", 8) == 0) i += 8;
  else if (strncmp(link, "holepunch://", 12) == 0) i += 12;
  else goto err;

  for (size_t j = 0; j < APPLING_KEY_MAX; i++, j++) {
    char c = link[i];

    if (c == '\0' || c == '/') {
      result->key[j] = '\0';
      break;
    }

    result->key[j] = c;
  }

  result->key[APPLING_KEY_MAX] = '\0';

  if (link[i] == '/') i++;
  else if (link[i] != '\0') goto err;

  for (size_t j = 0; j < APPLING_LINK_DATA_MAX; i++, j++) {
    char c = link[i];

    if (c == '\0') {
      result->data[j] = '\0';
      break;
    }

    result->data[j] = c;
  }

  result->data[APPLING_LINK_DATA_MAX] = '\0';

  if (link[i] != '\0') goto err;

  return 0;

err:
  return UV_EINVAL;
}
