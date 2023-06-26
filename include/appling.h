#ifndef APPLING_H
#define APPLING_H

#ifdef __cplusplus
extern "C" {
#endif

#include <fs.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <uv.h>

#include "appling/arch.h"
#include "appling/constants.h"
#include "appling/os.h"

#define APPLING_KEY_LEN       32
#define APPLING_LINK_DATA_MAX 4096

typedef uint8_t appling_key_t[APPLING_KEY_LEN];

typedef struct appling_platform_s appling_platform_t;
typedef struct appling_app_s appling_app_t;
typedef struct appling_link_s appling_link_t;
typedef struct appling_lock_s appling_lock_t;
typedef struct appling_resolve_s appling_resolve_t;
typedef struct appling_extract_s appling_extract_t;
typedef struct appling_bootstrap_s appling_bootstrap_t;
typedef struct appling_process_s appling_process_t;

typedef void (*appling_lock_cb)(appling_lock_t *req, int status);
typedef void (*appling_unlock_cb)(appling_lock_t *req, int status);
typedef void (*appling_resolve_cb)(appling_resolve_t *req, int status, const appling_platform_t *platform);
typedef void (*appling_extract_cb)(appling_extract_t *req, int status);
typedef void (*appling_bootstrap_cb)(appling_bootstrap_t *req, int status, const appling_app_t *app);
typedef void (*appling_exit_cb)(appling_process_t *process, int64_t exit_status, int term_signal);

struct appling_platform_s {
  char exe[PATH_MAX];
};

struct appling_app_s {
  appling_platform_t platform;
  char exe[PATH_MAX];
};

struct appling_link_s {
  appling_key_t key;
  char data[APPLING_LINK_DATA_MAX + 1 /* NULL */];
};

struct appling_lock_s {
  uv_loop_t *loop;

  appling_lock_cb on_lock;
  appling_unlock_cb on_unlock;

  fs_mkdir_t mkdir;
  fs_open_t open;
  fs_lock_t lock;
  fs_close_t close;

  char dir[PATH_MAX];

  uv_file file;

  int status;

  void *data;
};

struct appling_resolve_s {
  uv_loop_t *loop;

  appling_resolve_cb cb;

  fs_open_t open;
  fs_close_t close;
  fs_realpath_t realpath;
  fs_stat_t stat;
  fs_read_t read;

  char path[PATH_MAX];

  uv_file file;
  uv_buf_t buf;

  size_t bin_candidate;
  size_t exe_candidate;

  appling_platform_t platform;

  int status;

  void *data;
};

struct appling_extract_s {
  uv_loop_t *loop;
  uv_work_t req;

  char *archive;
  char *dest;

  appling_extract_cb cb;

  int status;

  void *data;
};

struct appling_bootstrap_s {
  uv_loop_t *loop;

  appling_bootstrap_cb cb;

  fs_swap_t swap;
  fs_rename_t rename;
  fs_rmdir_t rmdir;

  appling_extract_t extract;
  appling_resolve_t resolve;

  appling_app_t app;

  char exe_dir[PATH_MAX];

  char dir[PATH_MAX];

  uv_file file;
  uv_buf_t buf;

  int status;

  void *data;
};

struct appling_process_s {
  uv_process_t process;

  appling_exit_cb on_exit;
};

int
appling_parse (const char *link, appling_link_t *result);

int
appling_lock (uv_loop_t *loop, appling_lock_t *req, const char *dir, appling_lock_cb cb);

int
appling_unlock (uv_loop_t *loop, appling_lock_t *req, appling_unlock_cb cb);

int
appling_resolve (uv_loop_t *loop, appling_resolve_t *req, const char *dir, appling_resolve_cb cb);

int
appling_extract (uv_loop_t *loop, appling_extract_t *req, const char *archive, const char *dest, appling_extract_cb cb);

int
appling_bootstrap (uv_loop_t *loop, appling_bootstrap_t *req, const char *exe, const char *dir, const appling_platform_t *platform, appling_bootstrap_cb cb);

int
appling_launch (uv_loop_t *loop, appling_process_t *process, const appling_link_t *link, const appling_app_t *app, appling_exit_cb cb);

#ifdef __cplusplus
}
#endif

#endif // APPLING_H
