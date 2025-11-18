#include <assert.h>
#include <compact.h>
#include <path.h>
#include <stdint.h>
#include <stdlib.h>
#include <utf.h>
#include <uv.h>

#include "../include/appling.h"

static void *
appling_paths__on_alloc(size_t len, void *data) {
  return malloc(len * sizeof(appling_app_t));
}

static int
appling_paths__on_decode(compact_state_t *state, void *array, size_t i, void *data) {
  int err;

  appling_app_t *apps = (appling_app_t *) array;

  utf8_string_view_t path;
  err = compact_decode_utf8(state, &path);
  if (err < 0) return err;

  utf8_string_view_t id;
  err = compact_decode_utf8(state, &id);
  if (err < 0) return err;

  strncpy(apps[i].path, (char *) path.data, path.len);
  strncpy(apps[i].id, (char *) id.data, id.len);

  return 0;
}

static void
appling_paths__on_close(uv_fs_t *handle) {
  appling_paths_t *req = (appling_paths_t *) handle->data;

  int status = req->status;

  if (status >= 0) {
    req->cb(req, 0, req->apps, req->apps_len);
  } else {
    req->cb(req, status, NULL, 0);
  }

  free(req->apps);
}

static void
appling_paths__on_read(uv_fs_t *handle) {
  int err;

  appling_paths_t *req = (appling_paths_t *) handle->data;

  int status = handle->result;

  uv_fs_req_cleanup(handle);

  if (status >= 0) {
    compact_state_t state = {
      0,
      req->buf.len,
      (uint8_t *) req->buf.base,
    };

    int err;

    err = compact_decode_uint(&state, NULL);
    if (err < 0) goto done;

    err = compact_decode_array(&state, (void **) &req->apps, &req->apps_len, NULL, appling_paths__on_alloc, appling_paths__on_decode);
    if (err < 0) goto done;

  done:
    req->status = err;
  } else {
    req->status = status; // Propagate
  }

  free(req->buf.base);

  err = uv_fs_close(req->loop, &req->fs, req->file, appling_paths__on_close);
  assert(err == 0);
}

static void
applings_paths__on_stat(uv_fs_t *handle) {
  int err;

  appling_paths_t *req = (appling_paths_t *) handle->data;

  int status = handle->result;

  if (status >= 0) {
    const uv_stat_t *st = handle->ptr;

    req->buf = uv_buf_init(malloc(st->st_size), st->st_size);

    uv_fs_req_cleanup(handle);

    err = uv_fs_read(req->loop, &req->fs, req->file, &req->buf, 1, 0, appling_paths__on_read);
    assert(err == 0);
  } else {
    req->status = status; // Propagate

    uv_fs_req_cleanup(handle);

    err = uv_fs_close(req->loop, &req->fs, req->file, appling_paths__on_close);
    assert(err == 0);
  }
}

static void
appling_paths__on_open(uv_fs_t *handle) {
  int err;

  appling_paths_t *req = (appling_paths_t *) handle->data;

  int status = handle->result;

  uv_fs_req_cleanup(handle);

  if (status >= 0) {
    req->file = status;

    err = uv_fs_fstat(req->loop, &req->fs, req->file, applings_paths__on_stat);
    assert(err == 0);
  } else {
    req->cb(req, status, NULL, 0);
  }
}

int
appling_paths(uv_loop_t *loop, appling_paths_t *req, const char *dir, appling_paths_cb cb) {
  int err;

  req->loop = loop;
  req->cb = cb;
  req->status = 0;
  req->apps = NULL;
  req->apps_len = 0;
  req->fs.data = (void *) req;

  appling_path_t base;
  size_t path_len = sizeof(appling_path_t);

  if (dir && path_is_absolute(dir, path_behavior_system)) strcpy(base, dir);
  else if (dir) {
    appling_path_t cwd;
    size_t path_len = sizeof(appling_path_t);

    err = uv_cwd(cwd, &path_len);
    if (err < 0) return err;

    path_len = sizeof(appling_path_t);

    err = path_join(
      (const char *[]) {cwd, dir, NULL},
      base,
      &path_len,
      path_behavior_system
    );
    assert(err == 0);
  } else {
    appling_path_t homedir;
    size_t homedir_len = sizeof(appling_path_t);

    err = uv_os_homedir(homedir, &homedir_len);
    if (err < 0) return err;

    err = path_join(
      (const char *[]) {homedir, appling_platform_dir, NULL},
      base,
      &path_len,
      path_behavior_system
    );
    assert(err == 0);
  }

  path_len = sizeof(appling_path_t);

  err = path_join(
    (const char *[]) {base, "applings", NULL},
    req->path,
    &path_len,
    path_behavior_system
  );
  assert(err == 0);

  err = uv_fs_open(loop, &req->fs, req->path, O_RDONLY, 0, appling_paths__on_open);
  assert(err == 0);

  return 0;
}
