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
appling_launch_v0(const appling_launch_info_t *info) {
  int err;

  const appling_platform_t *platform = info->platform;

  appling_path_t file;

  size_t path_len = sizeof(appling_path_t);

  path_join(
    (const char *[]) {
      platform->path,
      "bin",
#if defined(APPLING_OS_WIN32)
    "pear-runtime.exe",
#elif defined(APPLING_OS_LINUX) || defined(APPLING_OS_DARWIN)
    "pear-runtime",
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

  char launch[7 /* pear:// */ + APPLING_ID_MAX + 1 /* / */ + APPLING_LINK_DATA_MAX + 1 /* NULL */] = {'\0'};

  strcat(launch, "pear://");
  strcat(launch, link->id);

  if (strlen(link->data)) {
    strcat(launch, "/");
    strcat(launch, link->data);
  }

  log_debug("appling_launch() launching link %s", launch);

  appling_path_t entry;

  strcpy(entry, file);

#if defined(APPLING_OS_WIN32)
  appling_path_t tmp;

  snprintf(tmp, sizeof(tmp), "\"%s\"", file);
  strcpy(file, tmp);

  snprintf(tmp, sizeof(tmp), "\"%s\"", appling);
  strcpy(appling, tmp);
#endif

  char *argv[9];

  size_t i = 0;

  argv[i++] = file;
  argv[i++] = "run";
  argv[i++] = "--appling";
  argv[i++] = appling;

  if (info->version >= 1 && info->name) {
    argv[i++] = "--app-name";
    argv[i++] = (char *) info->name;
  }

#if defined(APPLING_OS_WIN32) || defined(APPLING_OS_LINUX)
  argv[i++] = "--no-sandbox";
#endif

  argv[i++] = launch;
  argv[i] = NULL;

  return execv(entry, argv);
}
