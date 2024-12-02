#include <sys/types.h>
#include <unistd.h>

#include "../../include/appling.h"

int
appling_open(const appling_app_t *app, const char *argument) {
  pid_t pid = fork();

  if (pid < 0) return -1;

  if (pid == 0) execlp(app->path, argument, NULL);

  return 0;
}
