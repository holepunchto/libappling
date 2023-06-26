#include <hex.h>
#include <log.h>
#include <path.h>
#include <stdlib.h>
#include <string.h>
#include <utf.h>
#include <uv.h>

#include "../include/appling.h"

static inline bool
should_replace_platform (const appling_platform_t *platform, const appling_app_t *app) {
  return (
    memcmp(platform->key, app->platform.key, APPLING_KEY_LEN) != 0 || // Different platform
    platform->fork < app->platform.fork ||                            // Newer platform
    (
      platform->fork == app->platform.fork &&
      platform->len < app->platform.len
    )
  );
}

static void
on_resolve (appling_resolve_t *resolve, int status, const appling_platform_t *platform) {
  appling_bootstrap_t *req = (appling_bootstrap_t *) resolve->data;

  if (status >= 0) {
    memcpy(&req->app.platform, platform, sizeof(appling_platform_t));

    if (req->cb) req->cb(req, 0, &req->app);
  } else {
    if (req->cb) req->cb(req, status, NULL);
  }
}

static void
on_rmdir_tmp (fs_rmdir_t *fs_req, int status) {
  appling_bootstrap_t *req = (appling_bootstrap_t *) fs_req->data;

  status = req->status;

  if (status >= 0) {
    appling_resolve(req->loop, &req->resolve, req->dir, on_resolve);
  } else {
    if (req->cb) req->cb(req, status, NULL);
  }
}

static void
discard_tmp (appling_bootstrap_t *req) {
  char tmp[PATH_MAX];
  size_t path_len = PATH_MAX;

  path_join(
    (const char *[]){req->dir, "tmp", NULL},
    tmp,
    &path_len,
    path_behavior_system
  );

  fs_rmdir(req->loop, &req->rmdir, tmp, true, on_rmdir_tmp);
}

static void
on_rename (fs_rename_t *fs_req, int status) {
  appling_bootstrap_t *req = (appling_bootstrap_t *) fs_req->data;

  if (status >= 0) {
    req->status = 0;
  } else {
    req->status = status; // Propagate
  }

  discard_tmp(req);
}

static inline void
rename_platform (appling_bootstrap_t *req) {
  char to[PATH_MAX];
  size_t path_len = PATH_MAX;

  path_join(
    (const char *[]){req->dir, "platform", NULL},
    to,
    &path_len,
    path_behavior_system
  );

  char from[PATH_MAX];
  path_len = PATH_MAX;

  char app_key[65];
  size_t app_key_len = 65;

  hex_encode(req->app.key, APPLING_KEY_LEN, (utf8_t *) app_key, &app_key_len);

  path_join(
    (const char *[]){req->dir, "tmp", app_key, NULL},
    from,
    &path_len,
    path_behavior_system
  );

  log_debug("appling_bootstrap() renaming platform at %s", to);

  fs_rename(req->loop, &req->rename, from, to, on_rename);
}

static void
on_swap (fs_swap_t *fs_req, int status) {
  appling_bootstrap_t *req = (appling_bootstrap_t *) fs_req->data;

  if (status >= 0) {
    discard_tmp(req);
  } else {
    req->status = status; // Propagate

    rename_platform(req);
  }
}

static inline void
swap_platform (appling_bootstrap_t *req) {
  char to[PATH_MAX];
  size_t path_len = PATH_MAX;

  path_join(
    (const char *[]){req->dir, "platform", NULL},
    to,
    &path_len,
    path_behavior_system
  );

  char from[PATH_MAX];
  path_len = PATH_MAX;

  char app_key[65];
  size_t app_key_len = 65;

  hex_encode(req->app.key, APPLING_KEY_LEN, (utf8_t *) app_key, &app_key_len);

  path_join(
    (const char *[]){req->dir, "tmp", app_key, NULL},
    from,
    &path_len,
    path_behavior_system
  );

  log_debug("appling_bootstrap() swapping platform at %s", to);

  fs_swap(req->loop, &req->swap, from, to, on_swap);
}

static void
on_extract (appling_extract_t *extract, int status) {
  appling_bootstrap_t *req = (appling_bootstrap_t *) extract->data;

  if (status >= 0) {
    swap_platform(req);
  } else {
    req->status = status; // Propagate

    discard_tmp(req);
  }
}

static inline void
extract_platform (appling_bootstrap_t *req) {
  char archive[PATH_MAX];
  size_t path_len = PATH_MAX;

  path_join(
    (const char *[]){req->exe_dir, appling_platform_bundle, NULL},
    archive,
    &path_len,
    path_behavior_system
  );

  char dest[PATH_MAX];
  path_len = PATH_MAX;

  char app_key[65];
  size_t app_key_len = 65;

  hex_encode(req->app.key, APPLING_KEY_LEN, (utf8_t *) app_key, &app_key_len);

  path_join(
    (const char *[]){req->dir, "tmp", app_key, NULL},
    dest,
    &path_len,
    path_behavior_system
  );

  log_debug("appling_bootstrap() extracting platform archive to %s", dest);

  appling_extract(req->loop, &req->extract, archive, dest, on_extract);
}

static void
on_close_checkout (fs_close_t *fs_req, int status) {
  appling_bootstrap_t *req = (appling_bootstrap_t *) fs_req->data;

  if (req->status < 0) status = req->status;

  if (status >= 0) {
    if (!req->has_platform || should_replace_platform(&req->platform, &req->app)) {
      extract_platform(req);
    } else {
      log_debug("appling_bootstrap() using existing platform at %s", req->platform.exe);

      memcpy(&req->app.platform, &req->platform, sizeof(appling_platform_t));

      if (req->cb) req->cb(req, status, &req->app);
    }
  } else {
    if (req->cb) req->cb(req, status, NULL);
  }
}

static void
on_read_checkout (fs_read_t *fs_req, int status, size_t read) {
  appling_bootstrap_t *req = (appling_bootstrap_t *) fs_req->data;

  if (status >= 0) {
    req->buf.base[req->buf.len - 1] = '\0';

    size_t len;

    char platform_key[65];
    char app_key[65];

    sscanf(req->buf.base, "%i %i %64s %64s", &req->app.platform.fork, &req->app.platform.len, platform_key, app_key);

    len = APPLING_KEY_LEN;

    req->status = hex_decode((utf8_t *) platform_key, strlen(platform_key), req->app.platform.key, &len);

    if (req->status < 0) goto close;

    len = APPLING_KEY_LEN;

    req->status = hex_decode((utf8_t *) app_key, strlen(app_key), req->app.key, &len);

    if (req->status < 0) goto close;
  } else {
    req->status = status; // Propagate
  }

close:
  free(req->buf.base);

  fs_close(req->loop, &req->close, req->file, on_close_checkout);
}

static void
on_stat_checkout (fs_stat_t *fs_req, int status, const uv_stat_t *stat) {
  appling_bootstrap_t *req = (appling_bootstrap_t *) fs_req->data;

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
  appling_bootstrap_t *req = (appling_bootstrap_t *) fs_req->data;

  if (status >= 0) {
    req->file = file;

    fs_stat(req->loop, &req->stat, req->file, on_stat_checkout);
  } else {
    if (req->cb) req->cb(req, status, NULL);
  }
}

static void
open_checkout (appling_bootstrap_t *req) {
  char dir[PATH_MAX];
  size_t path_len = PATH_MAX;

  path_join(
    (const char *[]){req->exe_dir, appling_platform_bundle, "..", NULL},
    dir,
    &path_len,
    path_behavior_system
  );

  path_len = PATH_MAX;

  char checkout[PATH_MAX];
  path_len = PATH_MAX;

  path_join(
    (const char *[]){dir, "checkout", NULL},
    checkout,
    &path_len,
    path_behavior_system
  );

  log_debug("appling_bootstrap() opening checkout file at %s", checkout);

  fs_open(req->loop, &req->open, checkout, 0, O_RDONLY, on_open_checkout);
}

int
appling_bootstrap (uv_loop_t *loop, appling_bootstrap_t *req, const char *exe, const char *dir, const appling_platform_t *platform, appling_bootstrap_cb cb) {
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
  req->resolve.data = (void *) req;

  size_t path_len;

  if (exe) strcpy(req->app.exe, exe);
  else {
    path_len = PATH_MAX;
    uv_exepath(req->app.exe, &path_len);
  }

  if (dir) strcpy(req->dir, dir);
  else {
    char homedir[PATH_MAX];
    path_len = PATH_MAX;

    int err = uv_os_homedir(homedir, &path_len);
    if (err < 0) return err;

    path_len = PATH_MAX;

    path_join(
      (const char *[]){homedir, appling_platform_dir, NULL},
      req->dir,
      &path_len,
      path_behavior_system
    );
  }

  path_len = PATH_MAX;

  path_join(
    (const char *[]){req->app.exe, "..", NULL},
    req->exe_dir,
    &path_len,
    path_behavior_system
  );

  if (platform) memcpy(&req->platform, platform, sizeof(appling_platform_t));

  open_checkout(req);

  return 0;
}
