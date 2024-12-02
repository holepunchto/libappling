#include <compact.h>
#include <fs.h>
#include <log.h>
#include <path.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <utf.h>
#include <uv.h>

#include "../include/appling.h"

static void
appling_resolve__realpath(appling_resolve_t *req);

static void
appling_resolve__on_close(fs_close_t *fs_req, int status) {
  appling_resolve_t *req = (appling_resolve_t *) fs_req->data;

  if (req->status < 0) status = req->status;

  if (status >= 0) {
    if (req->cb) req->cb(req, 0);
  } else {
    size_t i = ++req->candidate;

    if (appling_platform_candidates[i]) appling_resolve__realpath(req);
    else if (req->cb) req->cb(req, status);
  }
}

static void
appling_resolve__on_read(fs_read_t *fs_req, int status, size_t read) {
  int err;

  appling_resolve_t *req = (appling_resolve_t *) fs_req->data;

  if (status >= 0) {
    compact_state_t state = {
      0,
      req->buf.len,
      (uint8_t *) req->buf.base,
    };

    uintmax_t length;
    err = compact_decode_uint(&state, &length);

    if (err < 0) {
      req->status = err; // Propagate
      goto close;
    }

    if (length < req->minimum_length) {
      req->status = -1;
      goto close;
    }

    req->platform->length = length;

    req->status = 0; // Reset
  } else {
    req->status = status; // Propagate
  }

close:
  free(req->buf.base);

  fs_close(req->loop, &req->close, req->file, appling_resolve__on_close);
}

static void
appling_resolve__on_stat(fs_stat_t *fs_req, int status, const uv_stat_t *stat) {
  appling_resolve_t *req = (appling_resolve_t *) fs_req->data;

  if (status >= 0) {
    size_t len = stat->st_size;

    req->buf = uv_buf_init(malloc(len), len);

    fs_read(req->loop, &req->read, req->file, &req->buf, 1, 0, appling_resolve__on_read);
  } else {
    req->status = status; // Propagate

    fs_close(req->loop, &req->close, req->file, appling_resolve__on_close);
  }
}

static void
appling_resolve__on_open(fs_open_t *fs_req, int status, uv_file file) {
  appling_resolve_t *req = (appling_resolve_t *) fs_req->data;

  if (status >= 0) {
    req->file = file;

    fs_stat(req->loop, &req->stat, req->file, appling_resolve__on_stat);
  } else {
    if (req->cb) req->cb(req, status);
  }
}

static void
appling_resolve__open(appling_resolve_t *req) {
  appling_path_t path;
  size_t path_len = sizeof(appling_path_t);

  path_join(
    (const char *[]) {req->platform->path, "..", "..", "length", NULL},
    path,
    &path_len,
    path_behavior_system
  );

  log_debug("appling_resolve() opening checkout file at %s", path);

  fs_open(req->loop, &req->open, path, 0, UV_FS_READ, appling_resolve__on_open);
}

static void
appling_resolve__on_realpath(fs_realpath_t *fs_req, int status, const char *path) {
  appling_resolve_t *req = (appling_resolve_t *) fs_req->data;

  if (status >= 0) {
    strcpy(req->platform->path, path);

    appling_resolve__open(req);
  } else {
    size_t i = ++req->candidate;

    if (appling_platform_candidates[i]) appling_resolve__realpath(req);
    else if (req->cb) req->cb(req, status);
  }
}

static void
appling_resolve__realpath(appling_resolve_t *req) {
  size_t i = req->candidate;

  appling_path_t path;
  size_t path_len = sizeof(appling_path_t);

  path_join(
    (const char *[]) {req->path, appling_platform_candidates[i], NULL},
    path,
    &path_len,
    path_behavior_system
  );

  log_debug("appling_resolve() accessing platform at %s", path);

  fs_realpath(req->loop, &req->realpath, path, appling_resolve__on_realpath);
}

int
appling_resolve(uv_loop_t *loop, appling_resolve_t *req, const char *dir, appling_platform_t *platform, uint64_t minimum_length, appling_resolve_cb cb) {
  req->loop = loop;
  req->cb = cb;
  req->platform = platform;
  req->minimum_length = minimum_length;
  req->candidate = 0;
  req->status = 0;
  req->realpath.data = (void *) req;
  req->open.data = (void *) req;
  req->stat.data = (void *) req;
  req->read.data = (void *) req;
  req->close.data = (void *) req;

  if (dir && path_is_absolute(dir, path_behavior_system)) strcpy(req->path, dir);
  else if (dir) {
    appling_path_t cwd;
    size_t path_len = sizeof(appling_path_t);

    int err = uv_cwd(cwd, &path_len);
    if (err < 0) return err;

    path_len = sizeof(appling_path_t);

    path_join(
      (const char *[]) {cwd, dir, NULL},
      req->path,
      &path_len,
      path_behavior_system
    );
  } else {
    appling_path_t homedir;
    size_t path_len = sizeof(appling_path_t);

    int err = uv_os_homedir(homedir, &path_len);
    if (err < 0) return err;

    path_len = sizeof(appling_path_t);

    path_join(
      (const char *[]) {homedir, appling_platform_dir, NULL},
      req->path,
      &path_len,
      path_behavior_system
    );
  }

  appling_resolve__realpath(req);

  return 0;
}
