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
on_rmdir_tmp (fs_rmdir_t *fs_req, int status) {
  holepunch_bootstrap_t *req = (holepunch_bootstrap_t *) fs_req->data;

  status = req->status;

  if (status >= 0) {
    req->cb(req, status, &req->app);
  } else {
    req->cb(req, status, NULL);
  }
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

  fs_rmdir(req->loop, &req->rmdir, path, true, on_rmdir_tmp);
}

static void
on_rename (fs_rename_t *fs_req, int status) {
  holepunch_bootstrap_t *req = (holepunch_bootstrap_t *) fs_req->data;

  if (status >= 0) {
    req->status = 0;
  } else {
    req->status = status; // Propagate
  }

  discard_tmp(req);
}

static inline void
rename_platform (holepunch_bootstrap_t *req) {
  char path[PATH_MAX];
  size_t path_len = PATH_MAX;

  path_join(
    (const char *[]){req->dir, "platform", NULL},
    path,
    &path_len,
    path_separator_system
  );

  fs_rename(req->loop, &req->rename, req->path, path, on_rename);
}

static void
on_swap (fs_swap_t *fs_req, int status) {
  holepunch_bootstrap_t *req = (holepunch_bootstrap_t *) fs_req->data;

  if (status >= 0) {
    discard_tmp(req);
  } else {
    req->status = status; // Propagate

    rename_platform(req);
  }
}

static inline void
swap_platform (holepunch_bootstrap_t *req) {
  char path[PATH_MAX];
  size_t path_len = PATH_MAX;

  path_join(
    (const char *[]){req->dir, "platform", NULL},
    path,
    &path_len,
    path_separator_system
  );

  fs_swap(req->loop, &req->swap, req->path, path, on_swap);
}

static void
on_extract (holepunch_extract_t *extract, int status) {
  holepunch_bootstrap_t *req = (holepunch_bootstrap_t *) extract->data;

  if (status >= 0) {
    swap_platform(req);
  } else {
    req->status = status; // Propagate

    discard_tmp(req);
  }
}

static inline void
extract_platform (holepunch_bootstrap_t *req) {
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
on_close_checkout (fs_close_t *fs_req, int status) {
  holepunch_bootstrap_t *req = (holepunch_bootstrap_t *) fs_req->data;

  if (req->status < 0) status = req->status;

  if (status >= 0) {
    if (!req->has_platform || should_replace_platform(&req->platform, &req->app)) {
      extract_platform(req);
    } else {
      memcpy(&req->app.platform, &req->platform, sizeof(holepunch_platform_t));

      req->cb(req, status, &req->app);
    }
  } else {
    req->cb(req, status, NULL);
  }
}

static void
on_read_checkout (fs_read_t *fs_req, int status, size_t read) {
  holepunch_bootstrap_t *req = (holepunch_bootstrap_t *) fs_req->data;

  if (status >= 0) {
    req->buf.base[req->buf.len - 1] = '\0';

    sscanf(req->buf.base, "%i %i %s %s", &req->app.platform.fork, &req->app.platform.len, req->app.platform.key, req->app.key);
  } else {
    req->status = status; // Propagate
  }

  free(req->buf.base);

  fs_close(req->loop, &req->close, req->file, on_close_checkout);
}

static void
on_stat_checkout (fs_stat_t *fs_req, int status, const uv_stat_t *stat) {
  holepunch_bootstrap_t *req = (holepunch_bootstrap_t *) fs_req->data;

  if (status >= 0) {
    size_t len = stat->st_size;

    req->buf = uv_buf_init(malloc(len), len);

    fs_read(req->loop, &req->read, req->file, &req->buf, 1, 0, on_read_checkout);
  } else {
    req->status = status; // Propagate

    fs_close(req->loop, &req->close, req->file, on_close_checkout);
  }
}

static void
on_open_checkout (fs_open_t *fs_req, int status, uv_file file) {
  holepunch_bootstrap_t *req = (holepunch_bootstrap_t *) fs_req->data;

  if (status >= 0) {
    req->file = file;

    fs_stat(req->loop, &req->stat, req->file, on_stat_checkout);
  } else {
    req->cb(req, status, NULL);
  }
}

static void
open_checkout (holepunch_bootstrap_t *req) {
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

  fs_open(req->loop, &req->open, req->path, 0, O_RDONLY, on_open_checkout);
}

int
holepunch_bootstrap (uv_loop_t *loop, holepunch_bootstrap_t *req, const char *exe, const char *dir, const holepunch_platform_t *platform, holepunch_bootstrap_cb cb) {
  req->loop = loop;
  req->cb = cb;
  req->status = 0;
  req->has_platform = platform != NULL;
  req->open.data = (void *) req;
  req->close.data = (void *) req;
  req->stat.data = (void *) req;
  req->read.data = (void *) req;
  req->swap.data = (void *) req;
  req->rename.data = (void *) req;
  req->rmdir.data = (void *) req;
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

  open_checkout(req);

  return 0;
}
