#include <fs.h>
#include <limits.h>
#include <path.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uv.h>

#include "../include/appling.h"

static void
on_close_checkout (fs_close_t *fs_req, int status) {
  appling_resolve_t *req = (appling_resolve_t *) fs_req->data;

  if (req->status < 0) status = req->status;

  if (status >= 0) {
    req->cb(req, 0, &req->platform);
  } else {
    req->cb(req, status, NULL);
  }
}

static void
on_read_checkout (fs_read_t *fs_req, int status, size_t read) {
  appling_resolve_t *req = (appling_resolve_t *) fs_req->data;

  if (status >= 0) {
    req->buf.base[read - 1] = '\0';

    sscanf(req->buf.base, "%i %i %64s", &req->platform.fork, &req->platform.len, req->platform.key);
  } else {
    req->status = status; // Propagate
  }

  free(req->buf.base);

  fs_close(req->loop, &req->close, req->file, on_close_checkout);
}

static void
on_stat_checkout (fs_stat_t *fs_req, int status, const uv_stat_t *stat) {
  appling_resolve_t *req = (appling_resolve_t *) fs_req->data;

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
  appling_resolve_t *req = (appling_resolve_t *) fs_req->data;

  if (status >= 0) {
    req->file = file;

    fs_stat(req->loop, &req->stat, req->file, on_stat_checkout);
  } else {
    req->cb(req, status, NULL);
  }
}

static inline void
open_checkout (appling_resolve_t *req) {
  char bin[PATH_MAX];

  strcpy(bin, req->platform.exe);

  size_t dirname = strlen(bin);

  bool is_bin = false;

  while (dirname > 4 && !is_bin) {
    is_bin = strcmp(APPLING_PATH_SEPARATOR "bin", &bin[dirname - 5]) == 0;

    path_dirname(bin, &dirname, path_separator_system);

    bin[dirname - 1] = '\0';
  }

  size_t path_len = PATH_MAX;

  path_join(
    (const char *[]){bin, "checkout", NULL},
    req->path,
    &path_len,
    path_separator_system
  );

  fs_open(req->loop, &req->open, req->path, 0, O_RDONLY, on_open_checkout);
}

static void
realpath_exe (appling_resolve_t *req);

static void
on_realpath_exe (fs_realpath_t *fs_req, int status, const char *path) {
  appling_resolve_t *req = (appling_resolve_t *) fs_req->data;

  if (status >= 0) {
    strcpy(req->platform.exe, path);

    open_checkout(req);
  } else {
    size_t i = ++req->exe_candidate;

    if (appling_exe_candidates[i]) realpath_exe(req);
    else req->cb(req, status, NULL);
  }
}

static void
realpath_exe (appling_resolve_t *req) {
  size_t i = req->bin_candidate;
  size_t j = req->exe_candidate;

  char path[PATH_MAX];
  size_t path_len = PATH_MAX;

  path_join(
    (const char *[]){req->path, appling_bin_candidates[i], appling_exe_candidates[j], NULL},
    path,
    &path_len,
    path_separator_system
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
    else req->cb(req, status, NULL);
  }
}

static void
open_bin (appling_resolve_t *req) {
  size_t i = req->bin_candidate;

  char path[PATH_MAX];
  size_t path_len = PATH_MAX;

  path_join(
    (const char *[]){req->path, appling_bin_candidates[i], NULL},
    path,
    &path_len,
    path_separator_system
  );

  fs_open(req->loop, &req->open, path, 0, O_RDONLY, on_open_bin);
}

int
appling_resolve (uv_loop_t *loop, appling_resolve_t *req, const char *dir, appling_resolve_cb cb) {
  req->loop = loop;
  req->cb = cb;
  req->bin_candidate = 0;
  req->exe_candidate = 0;
  req->status = 0;
  req->open.data = (void *) req;
  req->close.data = (void *) req;
  req->realpath.data = (void *) req;
  req->stat.data = (void *) req;
  req->read.data = (void *) req;

  if (dir) strcpy(req->path, dir);
  else {
    char homedir[PATH_MAX];
    size_t homedir_len = PATH_MAX;

    int err = uv_os_homedir(homedir, &homedir_len);
    if (err < 0) return err;

    size_t path_len = PATH_MAX;

    path_join(
      (const char *[]){homedir, appling_platform_dir, NULL},
      req->path,
      &path_len,
      path_separator_system
    );
  }

  open_bin(req);

  return 0;
}
