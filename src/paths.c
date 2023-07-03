#include <compact.h>
#include <fs.h>
#include <path.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <utf.h>
#include <uv.h>

#include "../include/appling.h"

static void *
on_alloc (size_t len, void *data) {
  return malloc(len * sizeof(appling_app_t));
}

static int
on_decode (compact_state_t *state, void *array, size_t i, void *data) {
  int err;

  appling_app_t *apps = (appling_app_t *) array;

  utf8_t *path;
  err = compact_decode_utf8(state, &path, NULL);
  if (err < 0) return err;

  err = compact_decode_fixed32(state, apps[i].key);
  if (err < 0) return err;

  strcpy(apps[i].path, (char *) path);

  free(path);

  return 0;
}

static void
on_close (fs_close_t *fs_req, int status) {
  appling_paths_t *req = (appling_paths_t *) fs_req->data;

  status = req->status;

  if (status >= 0) {
    if (req->cb) req->cb(req, 0, req->apps, req->apps_len);
  } else {
    if (req->cb) req->cb(req, status, NULL, 0);
  }

  free(req->apps);
}

static void
on_read (fs_read_t *fs_req, int status, size_t read) {
  appling_paths_t *req = (appling_paths_t *) fs_req->data;

  if (status >= 0) {
    compact_state_t state = {
      0,
      req->buf.len,
      (uint8_t *) req->buf.base,
    };

    int err;

    err = compact_decode_uint(&state, NULL);
    if (err < 0) goto done;

    err = compact_decode_array(&state, (void **) &req->apps, &req->apps_len, NULL, on_alloc, on_decode);
    if (err < 0) goto done;

  done:
    req->status = err;
  } else {
    req->status = status; // Propagate
  }

  free(req->buf.base);

  fs_close(req->loop, &req->close, req->file, on_close);
}

static void
on_stat (fs_stat_t *fs_req, int status, const uv_stat_t *st) {
  appling_paths_t *req = (appling_paths_t *) fs_req->data;

  if (status >= 0) {
    req->buf.len = st->st_size;
    req->buf.base = malloc(st->st_size);

    fs_read(req->loop, &req->read, req->file, &req->buf, 1, 0, on_read);
  } else {
    req->status = status; // Propagate

    fs_close(req->loop, &req->close, req->file, on_close);
  }
}

static void
on_open (fs_open_t *fs_req, int status, uv_file file) {
  appling_paths_t *req = (appling_paths_t *) fs_req->data;

  if (status >= 0) {
    req->file = file;

    fs_stat(req->loop, &req->stat, file, on_stat);
  } else {
    if (req->cb) req->cb(req, status, NULL, 0);
  }
}

int
appling_paths (uv_loop_t *loop, appling_paths_t *req, const char *dir, appling_paths_cb cb) {
  req->loop = loop;
  req->cb = cb;
  req->status = 0;
  req->apps = NULL;
  req->apps_len = 0;
  req->open.data = (void *) req;
  req->close.data = (void *) req;
  req->stat.data = (void *) req;
  req->read.data = (void *) req;

  appling_path_t base;
  size_t path_len = sizeof(appling_path_t);

  if (dir) strcpy(base, dir);
  else {
    appling_path_t homedir;
    size_t homedir_len = sizeof(appling_path_t);

    int err = uv_os_homedir(homedir, &homedir_len);
    if (err < 0) return err;

    path_join(
      (const char *[]){homedir, appling_platform_dir, NULL},
      base,
      &path_len,
      path_behavior_system
    );
  }

  path_len = sizeof(appling_path_t);

  path_join(
    (const char *[]){base, "applings", NULL},
    req->path,
    &path_len,
    path_behavior_system
  );

  fs_open(loop, &req->open, req->path, O_RDONLY, 0, on_open);

  return 0;
}
