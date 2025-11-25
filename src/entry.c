#include <assert.h>
#include <json.h>
#include <log.h>
#include <path.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "../include/appling.h"

#if defined(APPLING_OS_WIN32)
#define _CRT_SECURE_NO_WARNINGS
#include <process.h>
#include <wchar.h>
#include <windows.h>
#else
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#endif

typedef struct appling_line_parser_s appling_line_parser_t;

struct appling_line_parser_s {
  uint8_t buffer[1024];
  size_t len;
  bool skip;
};

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
appling_ready_v0(const appling_ready_info_t *info) {
  int err;

  appling_path_t file;

  size_t path_len = sizeof(appling_path_t);

  err = path_join(
    (const char *[]) {
      info->platform->path,
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
  assert(err == 0);

  char link[7 /* pear:// */ + APPLING_ID_MAX + 1 /* / */ + APPLING_LINK_DATA_MAX + 1 /* NULL */] = {'\0'};

  strcat(link, "pear://");
  strcat(link, info->link->id);

  if (strlen(info->link->data)) {
    strcat(link, "/");
    strcat(link, info->link->data);
  }

  log_debug("appling_ready() running for link %s", link);

  char *argv[5];

  size_t i = 0;

  argv[i++] = file;
  argv[i++] = "data";
  argv[i++] = "currents";
  argv[i++] = link;
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

  DWORD status;
  success = GetExitCodeProcess(pi.hProcess, &status);

  CloseHandle(pi.hProcess);
  CloseHandle(pi.hThread);

  return success ? status == 0 : -1;
#else
  pid_t pid = fork();

  if (pid < 0) return -1;

  if (pid == 0) {
    execv(file, argv);
    _exit(1);
  }

  int status;
  err = waitpid(pid, &status, 0);
  if (err < 0) return -1;

  return WIFEXITED(status) ? WEXITSTATUS(status) == 0 : -1;
#endif
}

static void
appling_preflight__on_line(const appling_preflight_info_t *info, uint8_t *line, size_t len) {
  int err;

  appling_progress_info_t progress = {.version = 0};

  json_t *value;
  err = json_decode_utf8(line, len, &value);
  if (err < 0) return;

  if (!json_is_object(value)) {
    json_deref(value);
    return;
  }

  json_t *tag = json_object_get_literal_utf8(value, (utf8_t *) "tag", -1);

  if (tag == NULL || !json_is_string(tag) || strcmp((const char *) json_string_value_utf8(tag), "stats") != 0) {
    if (tag) json_deref(tag);
    json_deref(value);
    return;
  }

  json_deref(tag);

  json_t *data = json_object_get_literal_utf8(value, (utf8_t *) "data", -1);

  if (data == NULL || !json_is_object(data)) {
    if (data) json_deref(data);
    json_deref(value);
    return;
  }

  json_deref(value);

  json_t *peers = json_object_get_literal_utf8(data, (utf8_t *) "peers", -1);

  if (peers == NULL || !json_is_number(peers)) {
    if (peers) json_deref(peers);
    json_deref(data);
    return;
  }

  progress.peers = (int64_t) json_number_value(peers);

  json_deref(peers);

  json_t *download = json_object_get_literal_utf8(data, (utf8_t *) "download", -1);

  if (download == NULL || !json_is_object(download)) {
    if (download) json_deref(download);
    json_deref(data);
    return;
  }

  json_t *download_speed = json_object_get_literal_utf8(download, (utf8_t *) "speed", -1);

  if (download_speed == NULL || !json_is_number(download_speed)) {
    if (download_speed) json_deref(download_speed);
    json_deref(download);
    json_deref(data);
    return;
  }

  progress.download_speed = json_number_value(download_speed);

  json_deref(download_speed);

  json_t *download_progress = json_object_get_literal_utf8(download, (utf8_t *) "progress", -1);

  if (download_progress == NULL || !json_is_number(download_progress)) {
    if (download_progress) json_deref(download_progress);
    json_deref(download);
    json_deref(data);
    return;
  }

  progress.download_progress = json_number_value(download_progress);

  json_deref(download_progress);

  json_t *downloaded_bytes = json_object_get_literal_utf8(download, (utf8_t *) "bytes", -1);

  if (downloaded_bytes == NULL || !json_is_number(downloaded_bytes)) {
    if (downloaded_bytes) json_deref(downloaded_bytes);
    json_deref(download);
    json_deref(data);
    return;
  }

  progress.downloaded_bytes = (int64_t) json_number_value(downloaded_bytes);

  json_deref(downloaded_bytes);

  json_t *downloaded_blocks = json_object_get_literal_utf8(download, (utf8_t *) "blocks", -1);

  if (downloaded_blocks == NULL || !json_is_number(downloaded_blocks)) {
    if (downloaded_blocks) json_deref(downloaded_blocks);
    json_deref(download);
    json_deref(data);
    return;
  }

  progress.downloaded_blocks = (int64_t) json_number_value(downloaded_blocks);

  json_deref(downloaded_blocks);
  json_deref(download);

  json_t *upload = json_object_get_literal_utf8(data, (utf8_t *) "upload", -1);

  if (upload == NULL || !json_is_object(upload)) {
    if (upload) json_deref(upload);
    json_deref(data);
    return;
  }

  json_t *upload_speed = json_object_get_literal_utf8(upload, (utf8_t *) "speed", -1);

  if (upload_speed == NULL || !json_is_number(upload_speed)) {
    if (upload_speed) json_deref(upload_speed);
    json_deref(upload);
    json_deref(data);
    return;
  }

  progress.upload_speed = json_number_value(upload_speed);

  json_deref(upload_speed);

  json_t *uploaded_bytes = json_object_get_literal_utf8(upload, (utf8_t *) "bytes", -1);

  if (uploaded_bytes == NULL || !json_is_number(uploaded_bytes)) {
    if (uploaded_bytes) json_deref(uploaded_bytes);
    json_deref(upload);
    json_deref(data);
    return;
  }

  progress.uploaded_bytes = (int64_t) json_number_value(uploaded_bytes);

  json_deref(uploaded_bytes);

  json_t *uploaded_blocks = json_object_get_literal_utf8(upload, (utf8_t *) "blocks", -1);

  if (uploaded_blocks == NULL || !json_is_number(uploaded_blocks)) {
    if (uploaded_blocks) json_deref(uploaded_blocks);
    json_deref(upload);
    json_deref(data);
    return;
  }

  progress.uploaded_blocks = (int64_t) json_number_value(uploaded_blocks);

  json_deref(uploaded_blocks);
  json_deref(upload);

  info->progress(&progress);
}

static void
appling_preflight__on_data(const appling_preflight_info_t *info, appling_line_parser_t *parser, uint8_t *data, size_t len) {
  if (info->progress == NULL) return;

  for (size_t i = 0; i < len; i++) {
    uint8_t c = data[i];

    if (c == '\n') {
      if (!parser->skip) appling_preflight__on_line(info, parser->buffer, parser->len);

      parser->len = 0;
      parser->skip = false;
    } else if (!parser->skip) {
      if (parser->len < sizeof(parser->buffer)) {
        parser->buffer[parser->len++] = c;
      } else {
        parser->skip = true;
      }
    }
  }
}

int
appling_preflight_v0(const appling_preflight_info_t *info) {
  int err;

  appling_path_t file;

  size_t path_len = sizeof(appling_path_t);

  err = path_join(
    (const char *[]) {
      info->platform->path,
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
  assert(err == 0);

  char link[7 /* pear:// */ + APPLING_ID_MAX + 1 /* / */ + APPLING_LINK_DATA_MAX + 1 /* NULL */] = {'\0'};

  strcat(link, "pear://");
  strcat(link, info->link->id);

  if (strlen(info->link->data)) {
    strcat(link, "/");
    strcat(link, info->link->data);
  }

  log_debug("appling_preflight() running for link %s", link);

  char *argv[6];

  size_t i = 0;

  argv[i++] = file;
  argv[i++] = "run";
  argv[i++] = "--trusted";
  argv[i++] = "--preflight";
  argv[i++] = link;
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

  DWORD status;
  success = GetExitCodeProcess(pi.hProcess, &status);

  CloseHandle(pi.hProcess);
  CloseHandle(pi.hThread);

  return success && status == 0 ? 0 : -1;
#else
  int fd[2];
  if (pipe(fd) < 0) return -1;

  pid_t pid = fork();

  if (pid < 0) {
    close(fd[0]);
    close(fd[1]);
    return -1;
  }

  if (pid == 0) {
    dup2(fd[1], STDOUT_FILENO);
    dup2(fd[1], STDERR_FILENO);

    close(fd[0]);
    close(fd[1]);

    execv(file, argv);
    _exit(1);
  }

  close(fd[1]);

  appling_line_parser_t parser = {{}, 0, false};

  uint8_t buffer[16384];

  for (;;) {
    ssize_t len = read(fd[0], buffer, sizeof(buffer));

    if (len < 0) {
      if (errno == EINTR) continue;
      close(fd[0]);
      break;
    }

    if (len == 0) break;

    appling_preflight__on_data(info, &parser, buffer, len);
  }

  int status;
  err = waitpid(pid, &status, 0);
  if (err < 0) return -1;

  return WIFEXITED(status) && WEXITSTATUS(status) == 0 ? 0 : -1;
#endif
}

int
appling_launch_v0(const appling_launch_info_t *info) {
  int err;

  appling_path_t file;

  size_t path_len = sizeof(appling_path_t);

  err = path_join(
    (const char *[]) {
      info->platform->path,
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
  assert(err == 0);

  const appling_app_t *app = info->app;

  appling_path_t appling;

#if defined(APPLING_OS_LINUX)
  char *appimage = getenv("APPIMAGE");

  strcpy(appling, appimage ? appimage : app->path);
#else
  strcpy(appling, app->path);
#endif

  log_debug("appling_launch() launching application shell %s", appling);

  char link[7 /* pear:// */ + APPLING_ID_MAX + 1 /* / */ + APPLING_LINK_DATA_MAX + 1 /* NULL */] = {'\0'};

  strcat(link, "pear://");
  strcat(link, info->link->id);

  if (strlen(info->link->data)) {
    strcat(link, "/");
    strcat(link, info->link->data);
  }

  log_debug("appling_launch() launching link %s", link);

  char *argv[8];

  size_t i = 0;

  argv[i++] = file;
  argv[i++] = "run";
  argv[i++] = "--trusted";
  argv[i++] = "--appling";
  argv[i++] = appling;

#if defined(APPLING_OS_WIN32) || defined(APPLING_OS_LINUX)
  argv[i++] = "--no-sandbox";
#endif

  argv[i++] = link;
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

  DWORD status;
  success = GetExitCodeProcess(pi.hProcess, &status);

  CloseHandle(pi.hProcess);
  CloseHandle(pi.hThread);

  return success && status == 0 ? 0 : -1;
#else
  return execv(file, argv);
#endif
}
