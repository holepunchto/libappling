#ifndef HOLEPUNCH_H
#define HOLEPUNCH_H

#ifdef __cplusplus
extern "C" {
#endif

#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <uv.h>

#include "holepunch/arch.h"
#include "holepunch/constants.h"
#include "holepunch/os.h"

#define HOLEPUNCH_KEY_MAX 64

typedef char holepunch_key_t[HOLEPUNCH_KEY_MAX + 1 /* NULL */];

typedef struct holepunch_platform_s holepunch_platform_t;
typedef struct holepunch_app_s holepunch_app_t;
typedef struct holepunch_resolve_s holepunch_resolve_t;
typedef struct holepunch_extract_s holepunch_extract_t;
typedef struct holepunch_bootstrap_s holepunch_bootstrap_t;

typedef void (*holepunch_resolve_cb)(holepunch_resolve_t *req, int status, const holepunch_platform_t *platform);
typedef void (*holepunch_extract_cb)(holepunch_extract_t *req, int status);
typedef void (*holepunch_bootstrap_cb)(holepunch_bootstrap_t *req, int status, const holepunch_app_t *app);

struct holepunch_platform_s {
  char exe[PATH_MAX];
  holepunch_key_t key;
  int fork;
  int len;
};

struct holepunch_app_s {
  holepunch_platform_t platform;
  holepunch_key_t key;
};

struct holepunch_resolve_s {
  uv_loop_t *loop;
  uv_fs_t req;

  holepunch_resolve_cb cb;

  char path[PATH_MAX];

  int fd;
  uv_buf_t buf;

  size_t bin_candidate;
  size_t exe_candidate;

  holepunch_platform_t platform;

  void *data;
};

struct holepunch_extract_s {
  uv_loop_t *loop;
  uv_work_t req;

  holepunch_extract_cb cb;

  const char *archive;
  const char *dest;

  int status;
  char *err;

  void *data;
};

struct holepunch_bootstrap_s {
  uv_loop_t *loop;
  uv_fs_t req;

  holepunch_bootstrap_cb cb;

  holepunch_extract_t extract;

  bool has_platform;
  holepunch_platform_t platform;

  holepunch_app_t app;

  char exe[PATH_MAX];
  char exe_dir[PATH_MAX];

  char dir[PATH_MAX];

  char path[PATH_MAX];

  int fd;
  uv_buf_t buf;

  int status;

  void *data;
};

int
holepunch_resolve (uv_loop_t *loop, holepunch_resolve_t *req, const char *dir, holepunch_resolve_cb cb);

int
holepunch_extract (uv_loop_t *loop, holepunch_extract_t *req, const char *archive, const char *dest, holepunch_extract_cb cb);

int
holepunch_bootstrap (uv_loop_t *loop, holepunch_bootstrap_t *req, const char *exe, const char *dir, const holepunch_platform_t *platform, holepunch_bootstrap_cb cb);

#ifdef __cplusplus
}
#endif

#endif // HOLEPUNCH_H
