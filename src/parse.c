#include <hex.h>
#include <stddef.h>
#include <string.h>
#include <utf.h>
#include <z32.h>

#include "../include/appling.h"

int
appling_parse (const char *link, appling_link_t *result) {
  size_t link_len = strlen(link);

  if (link_len == 0) goto err;

  size_t i = 0;

  if (strncmp(link, "pear://", 7) == 0) i += 7;
  else if (strncmp(link, "punch://", 8) == 0) i += 8;
  else goto err;

  size_t key_len = 0;

  for (; key_len < APPLING_KEY_MAX; i++) {
    char c = link[i];

    if (c == '\0' || c == '/') {
      result->key[key_len] = '\0';
      break;
    }

    result->key[key_len++] = c;
  }

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
