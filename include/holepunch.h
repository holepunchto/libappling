#ifndef HOLEPUNCH_H
#define HOLEPUNCH_H

#ifdef __cplusplus
extern "C" {
#endif

#include <fs.h>
#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <uv.h>

#include "holepunch/arch.h"
#include "holepunch/constants.h"
#include "holepunch/os.h"

#define HOLEPUNCH_KEY_MAX       64
#define HOLEPUNCH_LINK_DATA_MAX 4096

typedef char holepunch_key_t[HOLEPUNCH_KEY_MAX + 1 /* NULL */];

typedef struct holepunch_platform_s holepunch_platform_t;
typedef struct holepunch_app_s holepunch_app_t;
typedef struct holepunch_link_s holepunch_link_t;
typedef struct holepunch_lock_s holepunch_lock_t;
typedef struct holepunch_resolve_s holepunch_resolve_t;
typedef struct holepunch_extract_s holepunch_extract_t;
typedef struct holepunch_bootstrap_s holepunch_bootstrap_t;
typedef struct holepunch_process_s holepunch_process_t;

typedef void (*holepunch_lock_cb)(holepunch_lock_t *req, int status);
typedef void (*holepunch_unlock_cb)(holepunch_lock_t *req, int status);
typedef void (*holepunch_resolve_cb)(holepunch_resolve_t *req, int status, const holepunch_platform_t *platform);
typedef void (*holepunch_extract_cb)(holepunch_extract_t *req, int status);
typedef void (*holepunch_bootstrap_cb)(holepunch_bootstrap_t *req, int status, const holepunch_app_t *app);
typedef void (*holepunch_exit_cb)(holepunch_process_t *process, int64_t exit_status, int term_signal);

struct holepunch_platform_s {
  char exe[PATH_MAX];
  holepunch_key_t key;
  int fork;
  int len;
};

struct holepunch_app_s {
  holepunch_platform_t platform;
  char exe[PATH_MAX];
  holepunch_key_t key;
};

struct holepunch_link_s {
  holepunch_key_t key;
  char data[HOLEPUNCH_LINK_DATA_MAX + 1 /* NULL */];
};

struct holepunch_lock_s {
  uv_loop_t *loop;

  holepunch_lock_cb on_lock;
  holepunch_unlock_cb on_unlock;

  uv_file file;

  fs_open_t open;
  fs_lock_t lock;
  fs_close_t close;

  int status;

  void *data;
};

struct holepunch_resolve_s {
  uv_loop_t *loop;

  holepunch_resolve_cb cb;

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

  holepunch_platform_t platform;

  int status;

  void *data;
};

struct holepunch_extract_s {
  uv_loop_t *loop;
  uv_work_t req;

  char *archive;
  char *dest;

  holepunch_extract_cb cb;

  int status;

  void *data;
};

struct holepunch_bootstrap_s {
  uv_loop_t *loop;

  holepunch_bootstrap_cb cb;

  fs_open_t open;
  fs_close_t close;
  fs_stat_t stat;
  fs_read_t read;
  fs_swap_t swap;
  fs_rename_t rename;
  fs_rmdir_t rmdir;

  holepunch_extract_t extract;
  holepunch_resolve_t resolve;

  bool has_platform;
  holepunch_platform_t platform;
  holepunch_app_t app;

  char exe_dir[PATH_MAX];

  char dir[PATH_MAX];

  uv_file file;
  uv_buf_t buf;

  int status;

  void *data;
};

struct holepunch_process_s {
  uv_process_t process;

  holepunch_exit_cb on_exit;
};

int
holepunch_parse (const char *link, holepunch_link_t *result);

int
holepunch_lock (uv_loop_t *loop, holepunch_lock_t *req, const char *dir, holepunch_lock_cb cb);

int
holepunch_unlock (uv_loop_t *loop, holepunch_lock_t *req, holepunch_unlock_cb cb);

int
holepunch_resolve (uv_loop_t *loop, holepunch_resolve_t *req, const char *dir, holepunch_resolve_cb cb);

int
holepunch_extract (uv_loop_t *loop, holepunch_extract_t *req, const char *archive, const char *dest, holepunch_extract_cb cb);

int
holepunch_bootstrap (uv_loop_t *loop, holepunch_bootstrap_t *req, const char *exe, const char *dir, const holepunch_platform_t *platform, holepunch_bootstrap_cb cb);

int
holepunch_launch (uv_loop_t *loop, holepunch_process_t *process, const holepunch_link_t *link, const holepunch_app_t *app, holepunch_exit_cb cb);

#ifdef __cplusplus
}
#endif

#endif // HOLEPUNCH_H
