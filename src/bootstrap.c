#include <limits.h>
#include <path.h>
#include <stdlib.h>
#include <string.h>
#include <uv.h>

#include "../include/holepunch.h"

static inline bool
should_replace_platform (const holepunch_platform_t *platform, const holepunch_app_t *app) {
  return (
    strcmp(platform->key, app->platform.key) != 0 || // Different platform
    platform->fork < app->platform.fork ||           // Newer platform
    (
      platform->fork == app->platform.fork &&
      platform->len < app->platform.len
    )
  );
}

static void
on_rmdir (uv_fs_t *uv_req) {
  holepunch_bootstrap_t *req = (holepunch_bootstrap_t *) uv_req->data;

  // Disregard errors

  uv_fs_req_cleanup(uv_req);

  int status = req->status;

  if (status >= 0) req->cb(req, status, &req->app);
  else req->cb(req, status, NULL);
}

static void
discard_tmp (holepunch_bootstrap_t *req) {
  char path[PATH_MAX];
  size_t path_len = PATH_MAX;

  path_join(
    (const char *[]){req->dir, "tmp", NULL},
    path,
    &path_len,
    path_separator_system
  );

  uv_fs_rmdir(req->loop, &req->req, path, on_rmdir);
}

static void
on_rename (uv_fs_t *uv_req) {
  holepunch_bootstrap_t *req = (holepunch_bootstrap_t *) uv_req->data;

  // Disregard errors

  uv_fs_req_cleanup(uv_req);

  discard_tmp(req);
}

static void
on_extract (holepunch_extract_t *extract, int status) {
  holepunch_bootstrap_t *req = (holepunch_bootstrap_t *) extract->data;

  if (status < 0) req->status = status; // Propagate

  if (status >= 0) {
    char path[PATH_MAX];
    size_t path_len = PATH_MAX;

    path_join(
      (const char *[]){req->dir, "platform", NULL},
      path,
      &path_len,
      path_separator_system
    );

    uv_fs_rename(req->loop, &req->req, req->path, path, on_rename);
  } else {
    discard_tmp(req);
  }
}

static void
extract_bundled_platform (holepunch_bootstrap_t *req) {
  char path[PATH_MAX];
  size_t path_len = PATH_MAX;

  path_join(
    (const char *[]){req->exe_dir, holepunch_platform_bundle, NULL},
    path,
    &path_len,
    path_separator_system
  );

  path_len = PATH_MAX;

  path_join(
    (const char *[]){req->dir, "tmp", req->app.key, NULL},
    req->path,
    &path_len,
    path_separator_system
  );

  holepunch_extract(req->loop, &req->extract, path, req->path, on_extract);
}

static void
on_close_checkout (uv_fs_t *uv_req) {
  holepunch_bootstrap_t *req = (holepunch_bootstrap_t *) uv_req->data;

  // Disregard errors

  uv_fs_req_cleanup(uv_req);

  int status = req->status;

  if (status >= 0) {
    if (!req->has_platform || should_replace_platform(&req->platform, &req->app)) {
      extract_bundled_platform(req);
    } else {
      memcpy(&req->app.platform, &req->platform, sizeof(holepunch_platform_t));

      req->cb(req, status, &req->app);
    }
  } else {
    req->cb(req, status, NULL);
  }
}

static void
on_read_checkout (uv_fs_t *uv_req) {
  holepunch_bootstrap_t *req = (holepunch_bootstrap_t *) uv_req->data;

  int status = uv_req->result;
  uv_fs_req_cleanup(uv_req);

  if (status < 0) req->status = status; // Propagate

  if (status >= 0) {
    req->buf.base[req->buf.len - 1] = '\0';

    sscanf(req->buf.base, "%i %i %s %s", &req->app.platform.fork, &req->app.platform.len, req->app.platform.key, req->app.key);
  }

  free(req->buf.base);

  uv_fs_close(req->loop, &req->req, req->fd, on_close_checkout);
}

static void
on_open_checkout (uv_fs_t *uv_req) {
  holepunch_bootstrap_t *req = (holepunch_bootstrap_t *) uv_req->data;

  int status = uv_req->result;
  uv_fs_req_cleanup(uv_req);

  if (status >= 0) {
    req->fd = status;

    uv_fs_read(req->loop, &req->req, req->fd, &req->buf, 1, 0, on_read_checkout);
  } else {
    free(req->buf.base);

    req->cb(req, status, NULL);
  }
}

static void
on_stat_checkout (uv_fs_t *uv_req) {
  holepunch_bootstrap_t *req = (holepunch_bootstrap_t *) uv_req->data;

  int status = uv_req->result;

  if (status >= 0) {
    size_t len = uv_req->statbuf.st_size;

    uv_fs_req_cleanup(uv_req);

    req->buf = uv_buf_init(malloc(len), len);

    uv_fs_open(req->loop, &req->req, req->path, 0, O_RDONLY, on_open_checkout);
  } else {
    uv_fs_req_cleanup(uv_req);

    req->cb(req, status, NULL);
  }
}

static void
resolve_checkout (holepunch_bootstrap_t *req) {
  char path[PATH_MAX];
  size_t path_len = PATH_MAX;

  path_join(
    (const char *[]){req->exe_dir, holepunch_platform_bundle, NULL},
    path,
    &path_len,
    path_separator_system
  );

  path_dirname(path, &path_len, path_separator_system);

  path[path_len - 1] = '\0';

  path_len = PATH_MAX;

  path_join(
    (const char *[]){path, "checkout", NULL},
    req->path,
    &path_len,
    path_separator_system
  );

  uv_fs_stat(req->loop, &req->req, req->path, on_stat_checkout);
}

int
holepunch_bootstrap (uv_loop_t *loop, holepunch_bootstrap_t *req, const char *exe, const char *dir, const holepunch_platform_t *platform, holepunch_bootstrap_cb cb) {
  req->loop = loop;
  req->cb = cb;
  req->status = 0;
  req->has_platform = platform != NULL;
  req->req.data = (void *) req;
  req->extract.data = (void *) req;

  strcpy(req->exe, exe);
  strcpy(req->dir, dir);

  size_t dirname = 0;

  path_dirname(req->exe, &dirname, path_separator_system);

  strncpy(req->exe_dir, req->exe, dirname - 1);

  if (platform) memcpy(&req->platform, platform, sizeof(holepunch_platform_t));

  if (dir) strcpy(req->path, dir);
  else {
    char homedir[PATH_MAX];
    size_t homedir_len = PATH_MAX;

    int err = uv_os_homedir(homedir, &homedir_len);
    if (err < 0) return err;

    size_t path_len = PATH_MAX;

    path_join(
      (const char *[]){homedir, holepunch_dir, NULL},
      req->path,
      &path_len,
      path_separator_system
    );
  }

  resolve_checkout(req);

  return 0;
}
