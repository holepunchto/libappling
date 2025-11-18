#include <compact.h>
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
appling_resolve__on_close(uv_fs_t *handle) {
  appling_resolve_t *req = (appling_resolve_t *) handle->data;

  int status = req->status;

  uv_fs_req_cleanup(handle);

  if (status >= 0) {
    req->cb(req, 0);
  } else {
    size_t i = ++req->candidate;

    if (appling_platform_candidates[i]) {
      appling_resolve__realpath(req);
    } else {
      req->cb(req, status);
    }
  }
}

static void
appling_resolve__on_read(uv_fs_t *handle) {
  int err;

  appling_resolve_t *req = (appling_resolve_t *) handle->data;

  int status = handle->result;

  if (status >= 0) {
    compact_state_t state = {
      0,
      req->buf.len,
      (uint8_t *) req->buf.base,
    };

    uint8_t key[APPLING_KEY_LEN];
    err = compact_decode_fixed32(&state, key);

    if (err < 0) {
      req->status = err; // Propagate
      goto close;
    }

    uintmax_t length;
    err = compact_decode_uint(&state, &length);

    if (err < 0) {
      req->status = err; // Propagate
      goto close;
    }

    uintmax_t fork;
    err = compact_decode_uint(&state, &fork);

    if (err < 0) {
      req->status = err; // Propagate
      goto close;
    }

    if (memcmp(key, req->platform->key, APPLING_KEY_LEN) == 0) {
      if (length < req->platform->length || fork != req->platform->fork) {
        req->status = -1;
        goto close;
      }
    }

    utf8_string_view_t os;
    err = compact_decode_utf8(&state, &os);

    if (err < 0) {
      req->status = err; // Propagate
      goto close;
    }

    if (utf8_string_view_compare_literal(os, (const utf8_t *) APPLING_OS, -1) != 0) {
      req->status = -1;
      goto close;
    }

    utf8_string_view_t arch;
    err = compact_decode_utf8(&state, &arch);

    if (err < 0) {
      req->status = err; // Propagate
      goto close;
    }

    if (utf8_string_view_compare_literal(arch, (const utf8_t *) APPLING_ARCH, -1) != 0) {
      req->status = -1;
      goto close;
    }

    memcpy(req->platform->key, key, APPLING_KEY_LEN);

    req->platform->length = length;
    req->platform->fork = fork;

    req->status = 0; // Reset
  } else {
    req->status = status; // Propagate
  }

close:
  free(req->buf.base);

  err = uv_fs_close(req->loop, &req->fs, req->file, appling_resolve__on_close);
  assert(err == 0);
}

static void
appling_resolve__on_stat(uv_fs_t *handle) {
  int err;

  appling_resolve_t *req = (appling_resolve_t *) handle->data;

  int status = handle->result;

  if (status >= 0) {
    const uv_stat_t *st = handle->ptr;

    size_t len = st->st_size;

    req->buf = uv_buf_init(malloc(len), len);

    uv_fs_req_cleanup(handle);

    err = uv_fs_read(req->loop, &req->fs, req->file, &req->buf, 1, 0, appling_resolve__on_read);
    assert(err == 0);
  } else {
    req->status = status; // Propagate

    uv_fs_req_cleanup(handle);

    err = uv_fs_close(req->loop, &req->fs, req->file, appling_resolve__on_close);
    assert(err == 0);
  }
}

static void
appling_resolve__on_open(uv_fs_t *handle) {
  int err;

  appling_resolve_t *req = (appling_resolve_t *) handle->data;

  int status = handle->result;

  uv_fs_req_cleanup(handle);

  if (status >= 0) {
    req->file = status;

    err = uv_fs_fstat(req->loop, &req->fs, req->file, appling_resolve__on_stat);
    assert(err == 0);
  } else {
    req->cb(req, status);
  }
}

static void
appling_resolve__open(appling_resolve_t *req) {
  int err;

  appling_path_t path;
  size_t path_len = sizeof(appling_path_t);

  err = path_join(
    (const char *[]) {req->platform->path, "..", "..", "checkout", NULL},
    path,
    &path_len,
    path_behavior_system
  );
  assert(err == 0);

  log_debug("appling_resolve() opening checkout file at %s", path);

  err = uv_fs_open(req->loop, &req->fs, path, 0, UV_FS_READ, appling_resolve__on_open);
  assert(err == 0);
}

static void
appling_resolve__on_realpath(uv_fs_t *handle) {
  int err;

  appling_resolve_t *req = (appling_resolve_t *) handle->data;

  int status = handle->result;

  if (status >= 0) {
    const char *path = handle->ptr;

    strcpy(req->platform->path, path);

    uv_fs_req_cleanup(handle);

    appling_resolve__open(req);
  } else {
    uv_fs_req_cleanup(handle);

    size_t i = ++req->candidate;

    if (appling_platform_candidates[i]) {
      appling_resolve__realpath(req);
    } else {
      req->cb(req, status);
    }
  }
}

static void
appling_resolve__realpath(appling_resolve_t *req) {
  int err;

  size_t i = req->candidate;

  appling_path_t path;
  size_t path_len = sizeof(appling_path_t);

  err = path_join(
    (const char *[]) {req->path, appling_platform_candidates[i], NULL},
    path,
    &path_len,
    path_behavior_system
  );
  assert(err == 0);

  log_debug("appling_resolve() accessing platform at %s", path);

  err = uv_fs_realpath(req->loop, &req->fs, path, appling_resolve__on_realpath);
  assert(err == 0);
}

int
appling_resolve(uv_loop_t *loop, appling_resolve_t *req, const char *dir, appling_platform_t *platform, appling_resolve_cb cb) {
  int err;

  req->loop = loop;
  req->cb = cb;
  req->platform = platform;
  req->candidate = 0;
  req->status = 0;
  req->fs.data = (void *) req;

  if (dir && path_is_absolute(dir, path_behavior_system)) strcpy(req->path, dir);
  else if (dir) {
    appling_path_t cwd;
    size_t path_len = sizeof(appling_path_t);

    err = uv_cwd(cwd, &path_len);
    if (err < 0) return err;

    path_len = sizeof(appling_path_t);

    err = path_join(
      (const char *[]) {cwd, dir, NULL},
      req->path,
      &path_len,
      path_behavior_system
    );
    assert(err == 0);
  } else {
    appling_path_t homedir;
    size_t path_len = sizeof(appling_path_t);

    err = uv_os_homedir(homedir, &path_len);
    if (err < 0) return err;

    path_len = sizeof(appling_path_t);

    err = path_join(
      (const char *[]) {homedir, appling_platform_dir, NULL},
      req->path,
      &path_len,
      path_behavior_system
    );
    assert(err == 0);
  }

  appling_resolve__realpath(req);

  return 0;
}
