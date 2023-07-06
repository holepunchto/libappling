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
#include "appling/version.h"

#define APPLING_KEY_LEN       32
#define APPLING_LINK_DATA_MAX 4096

typedef char appling_path_t[4096 + 1 /* NULL */];

typedef uint8_t appling_key_t[APPLING_KEY_LEN];

typedef struct appling_platform_s appling_platform_t;
typedef struct appling_app_s appling_app_t;
typedef struct appling_link_s appling_link_t;
typedef struct appling_lock_s appling_lock_t;
typedef struct appling_resolve_s appling_resolve_t;
typedef struct appling_extract_s appling_extract_t;
typedef struct appling_bootstrap_s appling_bootstrap_t;
typedef struct appling_paths_s appling_paths_t;
typedef struct appling_launch_info_s appling_launch_info_t;

typedef void (*appling_lock_cb)(appling_lock_t *req, int status);
typedef void (*appling_unlock_cb)(appling_lock_t *req, int status);
typedef void (*appling_resolve_cb)(appling_resolve_t *req, int status);
typedef void (*appling_extract_cb)(appling_extract_t *req, int status);
typedef void (*appling_bootstrap_cb)(appling_bootstrap_t *req, int status);
typedef void (*appling_paths_cb)(appling_paths_t *req, int status, const appling_app_t *apps, size_t len);
typedef int (*appling_launch_cb)(const appling_launch_info_t *info);

struct appling_platform_s {
  appling_path_t path;
  appling_key_t key;
};

struct appling_app_s {
  appling_path_t path;
  appling_key_t key;
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

  appling_path_t dir;

  uv_file file;

  int status;

  void *data;
};

struct appling_resolve_s {
  uv_loop_t *loop;

  appling_resolve_cb cb;

  fs_realpath_t realpath;

  appling_path_t path;

  uv_file file;
  uv_buf_t buf;

  size_t candidate;

  appling_platform_t *platform;

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
  fs_mkdir_t mkdir;
  fs_merge_t merge;
  fs_symlink_t symlink;

  appling_extract_t extract;
  appling_resolve_t resolve;

  appling_key_t key;

  appling_path_t exe_dir;

  appling_path_t dir;

  int status;

  void *data;
};

struct appling_paths_s {
  uv_loop_t *loop;

  appling_paths_cb cb;

  fs_open_t open;
  fs_close_t close;
  fs_stat_t stat;
  fs_read_t read;

  appling_path_t path;

  appling_app_t *apps;
  size_t apps_len;

  uv_file file;
  uv_buf_t buf;

  int status;

  void *data;
};

struct appling_launch_info_s {
  int version;

  /**
   * The path to the object library from which the platform was launched.
   *
   * @since v0
   */
  const char *path;

  /**
   * The platform that was launched.
   *
   * @since v0
   */
  const appling_platform_t *platform;

  /**
   * The application bundle that is currently executing.
   *
   * @since v0
   */
  const appling_app_t *app;

  /**
   * The link to launch.
   *
   * @since v0
   */
  const appling_link_t *link;
};

int
appling_parse (const char *link, appling_link_t *result);

int
appling_lock (uv_loop_t *loop, appling_lock_t *req, const char *dir, appling_lock_cb cb);

int
appling_unlock (uv_loop_t *loop, appling_lock_t *req, appling_unlock_cb cb);

int
appling_resolve (uv_loop_t *loop, appling_resolve_t *req, const char *dir, appling_platform_t *platform, appling_resolve_cb cb);

int
appling_extract (uv_loop_t *loop, appling_extract_t *req, const char *archive, const char *dest, appling_extract_cb cb);

int
appling_bootstrap (uv_loop_t *loop, appling_bootstrap_t *req, const appling_key_t key, const char *exe, const char *dir, appling_bootstrap_cb cb);

int
appling_paths (uv_loop_t *loop, appling_paths_t *req, const char *dir, appling_paths_cb cb);

int
appling_launch (uv_loop_t *loop, const appling_platform_t *platform, const appling_app_t *app, const appling_link_t *link);

#ifdef __cplusplus
}
#endif

#endif // APPLING_H
