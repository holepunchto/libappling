#include <uv.h>

#include <shellapi.h> // Must be included after uv.h

#include "../../include/appling.h"

int
appling_open (const appling_app_t *app, const char *argument) {
  INT_PTR res = (INT_PTR) ShellExecute(NULL, "open", app->path, argument, NULL, 0);

  return res == 32 ? 0 : -1;
}
