#include <log.h>
#include <path.h>
#include <stdlib.h>
#include <string.h>

#include "../include/appling.h"

#if defined(APPLING_OS_WIN32)
#include <process.h>
#include <windows.h>
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

#define MAX_ARGS 9
  char *argv[MAX_ARGS];

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

#if defined(APPLING_OS_WIN32)
  int cmd_length = 0;
  for (int i = 0; i < MAX_ARGS && argv[i] != NULL; i++) {
    cmd_length = cmd_length + strlen(argv[i]) + 1; // +1 for space
  }
  cmd_length += 1; // +1 for final null terminator

  char cmd[cmd_length];
  cmd[0] = '\0'; // empty string

  for (int i = 0; i < MAX_ARGS && argv[i] != NULL; i++) {
    strncat(cmd, argv[i], strlen(argv[i]));
    if (argv[i + 1] != NULL) {
      strncat(cmd, " ", 1);
    }
  }

  STARTUPINFO si;
  PROCESS_INFORMATION pi;

  ZeroMemory(&si, sizeof(si));
  si.cb = sizeof(si);
  ZeroMemory(&pi, sizeof(pi));

  if (!CreateProcess(NULL, cmd, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
    printf("CreateProcess failed (%lu). \n", GetLastError());
    return 1;
  }
  WaitForSingleObject(pi.hProcess, INFINITE);
  CloseHandle(pi.hProcess);
  CloseHandle(pi.hThread);
  return 0;
#else
  return execv(entry, argv);
#endif
}
