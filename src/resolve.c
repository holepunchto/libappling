#include <fs.h>
#include <hex.h>
#include <log.h>
#include <path.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <utf.h>
#include <uv.h>

#include "../include/appling.h"

static void
realpath_platform (appling_resolve_t *req);

static void
on_realpath_platform (fs_realpath_t *fs_req, int status, const char *path) {
  appling_resolve_t *req = (appling_resolve_t *) fs_req->data;

  if (status >= 0) {
    strcpy(req->platform.path, path);

    if (req->cb) req->cb(req, 0, &req->platform);
  } else {
    size_t i = ++req->candidate;

    if (appling_platform_candidates[i]) realpath_platform(req);
    else if (req->cb) req->cb(req, status, NULL);
  }
}

static void
realpath_platform (appling_resolve_t *req) {
  size_t i = req->candidate;

  appling_path_t path;
  size_t path_len = sizeof(appling_path_t);

  path_join(
    (const char *[]){req->path, appling_platform_candidates[i], NULL},
    path,
    &path_len,
    path_behavior_system
  );

  log_debug("appling_resolve() accessing platform at %s", path);

  fs_realpath(req->loop, &req->realpath, path, on_realpath_platform);
}

int
appling_resolve (uv_loop_t *loop, appling_resolve_t *req, const char *dir, appling_resolve_cb cb) {
  req->loop = loop;
  req->cb = cb;
  req->candidate = 0;
  req->status = 0;
  req->realpath.data = (void *) req;

  if (dir) strcpy(req->path, dir);
  else {
    appling_path_t homedir;
    size_t homedir_len = sizeof(appling_path_t);

    int err = uv_os_homedir(homedir, &homedir_len);
    if (err < 0) return err;

    size_t path_len = sizeof(appling_path_t);

    path_join(
      (const char *[]){homedir, appling_platform_dir, NULL},
      req->path,
      &path_len,
      path_behavior_system
    );
  }

  realpath_platform(req);

  return 0;
}
