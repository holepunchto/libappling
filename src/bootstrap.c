#include <hex.h>
#include <log.h>
#include <path.h>
#include <stdlib.h>
#include <string.h>
#include <utf.h>
#include <uv.h>

#include "../include/appling.h"

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
  appling_path_t tmp;
  size_t path_len = sizeof(appling_path_t);

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
  appling_path_t to;
  size_t path_len = sizeof(appling_path_t);

  path_join(
    (const char *[]){req->dir, "platform", NULL},
    to,
    &path_len,
    path_behavior_system
  );

  appling_path_t from;
  path_len = sizeof(appling_path_t);

  path_join(
    (const char *[]){req->dir, "tmp", NULL},
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
  appling_path_t to;
  size_t path_len = sizeof(appling_path_t);

  path_join(
    (const char *[]){req->dir, "platform", NULL},
    to,
    &path_len,
    path_behavior_system
  );

  appling_path_t from;
  path_len = sizeof(appling_path_t);

  path_join(
    (const char *[]){req->dir, "tmp", NULL},
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
  appling_path_t archive;
  size_t path_len = sizeof(appling_path_t);

  path_join(
    (const char *[]){req->exe_dir, appling_platform_bundle, NULL},
    archive,
    &path_len,
    path_behavior_system
  );

  appling_path_t dest;
  path_len = sizeof(appling_path_t);

  path_join(
    (const char *[]){req->dir, "tmp", NULL},
    dest,
    &path_len,
    path_behavior_system
  );

  log_debug("appling_bootstrap() extracting platform archive to %s", dest);

  appling_extract(req->loop, &req->extract, archive, dest, on_extract);
}

int
appling_bootstrap (uv_loop_t *loop, appling_bootstrap_t *req, const char *exe, const char *dir, const appling_platform_t *platform, appling_bootstrap_cb cb) {
  req->loop = loop;
  req->cb = cb;
  req->status = 0;
  req->swap.data = (void *) req;
  req->rename.data = (void *) req;
  req->rmdir.data = (void *) req;
  req->extract.data = (void *) req;
  req->resolve.data = (void *) req;

  size_t path_len;

  if (exe) strcpy(req->app.exe, exe);
  else {
    path_len = sizeof(appling_path_t);
    uv_exepath(req->app.exe, &path_len);
  }

  if (dir) strcpy(req->dir, dir);
  else {
    appling_path_t homedir;
    path_len = sizeof(appling_path_t);

    int err = uv_os_homedir(homedir, &path_len);
    if (err < 0) return err;

    path_len = sizeof(appling_path_t);

    path_join(
      (const char *[]){homedir, appling_platform_dir, NULL},
      req->dir,
      &path_len,
      path_behavior_system
    );
  }

  path_len = sizeof(appling_path_t);

  path_join(
    (const char *[]){req->app.exe, "..", NULL},
    req->exe_dir,
    &path_len,
    path_behavior_system
  );

  if (platform) {
    on_resolve(&req->resolve, 0, platform);
  } else {
    extract_platform(req);
  }

  return 0;
}
