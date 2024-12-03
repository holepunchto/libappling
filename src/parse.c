#include <stddef.h>
#include <string.h>

#include "../include/appling.h"

int
appling_parse(const char *link, appling_link_t *result) {
  size_t link_len = strlen(link);

  if (link_len == 0) goto err;

  size_t i = 0;

  if (strncmp(link, "pear://", 7) == 0) i += 7;
  else if (strncmp(link, "punch://", 8) == 0) i += 8;
  else goto err;

  for (size_t j = 0; j < APPLING_ID_MAX; i++, j++) {
    char c = link[i];

    if (c == '\0' || c == '/') {
      result->id[j] = '\0';
      break;
    }

    result->id[j] = c;
  }

  if (strlen(result->id) == 0) goto err;

  result->id[APPLING_ID_MAX] = '\0';

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
