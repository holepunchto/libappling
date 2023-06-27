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
realpath_exe (appling_resolve_t *req);

static void
on_realpath_exe (fs_realpath_t *fs_req, int status, const char *path) {
  appling_resolve_t *req = (appling_resolve_t *) fs_req->data;

  if (status >= 0) {
    strcpy(req->platform.exe, path);

    if (req->cb) req->cb(req, 0, &req->platform);
  } else {
    if (req->cb) req->cb(req, status, NULL);
  }
}

static void
realpath_exe (appling_resolve_t *req) {
  size_t i = req->bin_candidate;

  appling_path_t path;
  size_t path_len = sizeof(appling_path_t);

  path_join(
    (const char *[]){req->path, appling_bin_candidates[i], appling_platform_exe, NULL},
    path,
    &path_len,
    path_behavior_system
  );

  fs_realpath(req->loop, &req->realpath, path, on_realpath_exe);
}

static void
on_close_bin (fs_close_t *fs_req, int status) {
  appling_resolve_t *req = (appling_resolve_t *) fs_req->data;

  realpath_exe(req);
}

static void
open_bin (appling_resolve_t *req);

static void
on_open_bin (fs_open_t *fs_req, int status, uv_file file) {
  appling_resolve_t *req = (appling_resolve_t *) fs_req->data;

  if (status >= 0) {
    fs_close(req->loop, &req->close, file, on_close_bin);
  } else {
    size_t i = ++req->bin_candidate;

    if (appling_bin_candidates[i]) open_bin(req);
    else if (req->cb) req->cb(req, status, NULL);
  }
}

static void
open_bin (appling_resolve_t *req) {
  size_t i = req->bin_candidate;

  appling_path_t path;
  size_t path_len = sizeof(appling_path_t);

  path_join(
    (const char *[]){req->path, appling_bin_candidates[i], NULL},
    path,
    &path_len,
    path_behavior_system
  );

  log_debug("appling_resolve() opening platform binary at %s", path);

  fs_open(req->loop, &req->open, path, 0, O_RDONLY, on_open_bin);
}

int
appling_resolve (uv_loop_t *loop, appling_resolve_t *req, const char *dir, appling_resolve_cb cb) {
  req->loop = loop;
  req->cb = cb;
  req->bin_candidate = 0;
  req->status = 0;
  req->open.data = (void *) req;
  req->close.data = (void *) req;
  req->realpath.data = (void *) req;
  req->stat.data = (void *) req;
  req->read.data = (void *) req;

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

  open_bin(req);

  return 0;
}
