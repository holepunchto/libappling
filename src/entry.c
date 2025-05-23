#include <log.h>
#include <path.h>
#include <stdlib.h>
#include <string.h>

#include "../include/appling.h"

#if defined(APPLING_OS_WIN32)
#define _CRT_SECURE_NO_WARNINGS
#include <process.h>
#include <wchar.h>
#include <windows.h>
#else
#include <unistd.h>
#endif

#if defined(APPLING_OS_WIN32)
static inline int32_t
appling__wtf8_decode1(const char **input) {
  uint32_t code_point;
  uint8_t b1;
  uint8_t b2;
  uint8_t b3;
  uint8_t b4;

  b1 = **input;
  if (b1 <= 0x7F) return b1;
  if (b1 < 0xC2) return -1;
  code_point = b1;

  b2 = *++*input;
  if ((b2 & 0xC0) != 0x80) return -1;
  code_point = (code_point << 6) | (b2 & 0x3F);
  if (b1 <= 0xDF) return 0x7FF & code_point;

  b3 = *++*input;
  if ((b3 & 0xC0) != 0x80) return -1;
  code_point = (code_point << 6) | (b3 & 0x3F);
  if (b1 <= 0xEF) return 0xFFFF & code_point;

  b4 = *++*input;
  if ((b4 & 0xC0) != 0x80) return -1;
  code_point = (code_point << 6) | (b4 & 0x3F);
  if (b1 <= 0xF4) {
    code_point &= 0x1FFFFF;
    if (code_point <= 0x10FFFF) return code_point;
  }

  return -1;
}

static inline ssize_t
appling__utf16_length_from_wtf8(const char *source) {
  size_t target_len = 0;
  int32_t code_point;

  do {
    code_point = appling__wtf8_decode1(&source);

    if (code_point < 0) return -1;
    if (code_point > 0xFFFF) target_len++;

    target_len++;
  } while (*source++);

  return target_len;
}

static inline void
appling__wtf8_to_utf16(const char *source, uint16_t *target) {
  int32_t code_point;

  do {
    code_point = appling__wtf8_decode1(&source);

    if (code_point > 0xFFFF) {
      *target++ = (((code_point - 0x10000) >> 10) + 0xD800);
      *target++ = ((code_point - 0x10000) & 0x3FF) + 0xDC00;
    } else {
      *target++ = code_point;
    }
  } while (*source++);
}

static inline int
appling__utf8_to_utf16(const char *utf8, WCHAR **result) {
  int len;
  len = appling__utf16_length_from_wtf8(utf8);
  if (len < 0) return -1;

  WCHAR *utf16 = malloc(len * sizeof(WCHAR));
  assert(utf16);

  appling__wtf8_to_utf16(utf8, utf16);

  *result = utf16;

  return 0;
}

static inline WCHAR *
appling__quote_argument(const WCHAR *source, WCHAR *target) {
  size_t len = wcslen(source);
  size_t i;
  int quote_hit;
  WCHAR *start;

  if (len == 0) {
    *(target++) = L'"';
    *(target++) = L'"';

    return target;
  }

  if (NULL == wcspbrk(source, L" \t\"")) {
    wcsncpy(target, source, len);
    target += len;

    return target;
  }

  if (NULL == wcspbrk(source, L"\"\\")) {
    *(target++) = L'"';
    wcsncpy(target, source, len);
    target += len;
    *(target++) = L'"';

    return target;
  }

  *(target++) = L'"';
  start = target;
  quote_hit = 1;

  for (i = len; i > 0; --i) {
    *(target++) = source[i - 1];

    if (quote_hit && source[i - 1] == L'\\') {
      *(target++) = L'\\';
    } else if (source[i - 1] == L'"') {
      quote_hit = 1;
      *(target++) = L'\\';
    } else {
      quote_hit = 0;
    }
  }

  target[0] = L'\0';
  _wcsrev(start);
  *(target++) = L'"';

  return target;
}

static inline int
appling__argv_to_command_line(const char *const *args, WCHAR **result) {
  const char *const *arg;
  WCHAR *dst = NULL;
  WCHAR *tmp = NULL;
  size_t dst_len = 0;
  size_t tmp_len = 0;
  WCHAR *pos;
  int arg_count = 0;

  for (arg = args; *arg; arg++) {
    ssize_t arg_len = appling__utf16_length_from_wtf8(*arg);

    if (arg_len < 0) return arg_len;

    dst_len += arg_len;

    if ((size_t) arg_len > tmp_len) tmp_len = arg_len;

    arg_count++;
  }

  dst_len = dst_len * 2 + arg_count * 2;

  dst = malloc(dst_len * sizeof(WCHAR));
  assert(dst);

  tmp = malloc(tmp_len * sizeof(WCHAR));
  assert(tmp);

  pos = dst;

  for (arg = args; *arg; arg++) {
    ssize_t arg_len = appling__utf16_length_from_wtf8(*arg);

    appling__wtf8_to_utf16(*arg, tmp);

    pos = appling__quote_argument(tmp, pos);

    *pos++ = *(arg + 1) ? L' ' : L'\0';
  }

  free(tmp);

  *result = dst;

  return 0;
}
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
#if defined(APPLING_OS_DARWIN) || defined(APPLING_OS_LINUX)
      "pear-runtime",
#elif defined(APPLING_OS_WIN32)
      "pear-runtime.exe",
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

#if defined(APPLING_OS_WIN32)
  STARTUPINFOW si;
  ZeroMemory(&si, sizeof(si));

  si.cb = sizeof(si);

  PROCESS_INFORMATION pi;
  ZeroMemory(&pi, sizeof(pi));

  WCHAR *application_name;
  err = appling__utf8_to_utf16(file, &application_name);
  if (err < 0) return err;

  WCHAR *command_line;
  err = appling__argv_to_command_line((const char *const *) argv, &command_line);
  if (err < 0) {
    free(application_name);

    return err;
  }

  BOOL success = CreateProcessW(
    application_name,
    command_line,
    NULL,
    NULL,
    FALSE,
    CREATE_NO_WINDOW,
    NULL,
    NULL,
    &si,
    &pi
  );

  free(application_name);
  free(command_line);

  if (!success) return -1;

  WaitForSingleObject(pi.hProcess, INFINITE);

  CloseHandle(pi.hProcess);
  CloseHandle(pi.hThread);

  return 0;
#else
  return execv(entry, argv);
#endif
}
