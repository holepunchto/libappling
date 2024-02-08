#include <log.h>
#include <path.h>
#include <stdlib.h>
#include <string.h>

#include "../include/appling.h"

#if defined(APPLING_OS_WIN32)
#include <process.h>
#else
#include <unistd.h>
#endif

int
appling_launch_v0 (const appling_launch_info_t *info) {
  int err;

  const appling_platform_t *platform = info->platform;

  appling_path_t file;
  size_t path_len = sizeof(appling_path_t);

  path_join(
    (const char *[]) {
      platform->path,
        "bin",
#if defined(APPLING_OS_LINUX)
        "pear-runtime-app/pear-runtime",
#elif defined(APPLING_OS_DARWIN)
        "Pear Runtime.app/Contents/MacOS/Pear Runtime",
#elif defined(APPLING_OS_WIN32)
        "pear-runtime-app\Pear Runtime.exe",
#else
#error Unsupported operating system
#endif
        NULL,
    },
    file,
    &path_len,
    path_behavior_system
  );

  const appling_app_t *app = info->app;

  appling_path_t appling;

#if defined(APPLING_OS_LINUX)
  char *appimage = getenv("APPIMAGE");

  strcpy(appling, appimage ? appimage : app->path);
#else
  strcpy(appling, app->path);
#endif

  log_debug("appling_launch() launching application shell %s", appling);

  const appling_link_t *link = info->link;

  char launch[7 /* pear:// */ + APPLING_KEY_MAX + 1 /* / */ + APPLING_LINK_DATA_MAX + 1 /* NULL */] = {'\0'};

  strcat(launch, "pear://");
  strcat(launch, link->key);

  if (strlen(link->data)) {
    strcat(launch, "/");
    strcat(launch, link->data);
  }

  log_debug("appling_launch() launching link %s", launch);

  char *args[] = {file, "--run", launch, "--appling", appling, NULL};

#if defined(APPLING_OS_WIN32)
  err = _execv(file, args);
#else
  err = execv(file, args);
#endif

  return err;
}
