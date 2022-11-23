#include <limits.h>
#include <path.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uv.h>

#include "../include/holepunch.h"

static void
on_close_checkout (uv_fs_t *uv_req) {
  holepunch_resolve_t *req = (holepunch_resolve_t *) uv_req->data;

  int status = uv_req->result;
  uv_fs_req_cleanup(uv_req);

  if (status >= 0) {
    req->cb(req, 0, &req->platform);
  } else {
    req->cb(req, status, NULL);
  }
}

static void
on_read_checkout (uv_fs_t *uv_req) {
  holepunch_resolve_t *req = (holepunch_resolve_t *) uv_req->data;

  int status = uv_req->result;
  uv_fs_req_cleanup(uv_req);

  if (status >= 0) {
    req->buf.base[req->buf.len - 1] = '\0';

    sscanf(req->buf.base, "%i %i %s64", &req->platform.fork, &req->platform.len, req->platform.key);
  }

  free(req->buf.base);

  uv_fs_close(req->loop, &req->req, req->fd, on_close_checkout);
}

static void
on_open_checkout (uv_fs_t *uv_req) {
  holepunch_resolve_t *req = (holepunch_resolve_t *) uv_req->data;

  int status = uv_req->result;
  uv_fs_req_cleanup(uv_req);

  if (status >= 0) {
    req->fd = status;

    uv_fs_read(req->loop, &req->req, req->fd, &req->buf, 1, 0, on_read_checkout);
  } else {
    free(req->buf.base);

    req->cb(req, status, NULL);
  }
}

static void
on_stat_checkout (uv_fs_t *uv_req) {
  holepunch_resolve_t *req = (holepunch_resolve_t *) uv_req->data;

  int status = uv_req->result;

  if (status >= 0) {
    size_t len = uv_req->statbuf.st_size;

    uv_fs_req_cleanup(uv_req);

    req->buf = uv_buf_init(malloc(len), len);

    uv_fs_open(req->loop, &req->req, req->path, 0, O_RDONLY, on_open_checkout);
  } else {
    uv_fs_req_cleanup(uv_req);

    req->cb(req, status, NULL);
  }
}

static void
resolve_checkout (holepunch_resolve_t *req) {
  char bin[PATH_MAX];

  strcpy(bin, req->platform.exe);

  size_t dirname = strlen(bin);

  bool is_bin = false;

  while (dirname > 4 && !is_bin) {
    is_bin = strcmp(HOLEPUNCH_PATH_SEPARATOR "bin", &bin[dirname - 5]) == 0;

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

  uv_fs_stat(req->loop, &req->req, req->path, on_stat_checkout);
}

static void
realpath_exe_candidate (holepunch_resolve_t *req);

static void
on_realpath_exe_candidate (uv_fs_t *uv_req) {
  holepunch_resolve_t *req = (holepunch_resolve_t *) uv_req->data;

  int status = uv_req->result;

  if (status >= 0) {
    strcpy(req->platform.exe, (char *) uv_req->ptr);

    uv_fs_req_cleanup(uv_req);

    resolve_checkout(req);
  } else {
    uv_fs_req_cleanup(uv_req);

    size_t i = ++req->exe_candidate;

    if (holepunch_exe_candidates[i]) realpath_exe_candidate(req);
    else req->cb(req, status, NULL);
  }
}

static void
realpath_exe_candidate (holepunch_resolve_t *req) {
  size_t i = req->bin_candidate;
  size_t j = req->exe_candidate;

  char path[PATH_MAX];
  size_t path_len = PATH_MAX;

  path_join(
    (const char *[]){req->path, holepunch_bin_candidates[i], holepunch_exe_candidates[j], NULL},
    path,
    &path_len,
    path_separator_system
  );

  uv_fs_realpath(req->loop, &req->req, path, on_realpath_exe_candidate);
}

static void
stat_bin_candidate (holepunch_resolve_t *req);

static void
on_stat_bin_candidate (uv_fs_t *uv_req) {
  holepunch_resolve_t *req = (holepunch_resolve_t *) uv_req->data;

  int status = uv_req->result;
  uv_fs_req_cleanup(uv_req);

  if (status >= 0) {
    realpath_exe_candidate(req);
  } else {
    size_t i = ++req->bin_candidate;

    if (holepunch_bin_candidates[i]) stat_bin_candidate(req);
    else req->cb(req, status, NULL);
  }
}

static void
stat_bin_candidate (holepunch_resolve_t *req) {
  size_t i = req->bin_candidate;

  char path[PATH_MAX];
  size_t path_len = PATH_MAX;

  path_join(
    (const char *[]){req->path, holepunch_bin_candidates[i], NULL},
    path,
    &path_len,
    path_separator_system
  );

  uv_fs_stat(req->loop, &req->req, path, on_stat_bin_candidate);
}

int
holepunch_resolve (uv_loop_t *loop, holepunch_resolve_t *req, const char *dir, holepunch_resolve_cb cb) {
  req->loop = loop;
  req->cb = cb;
  req->bin_candidate = 0;
  req->exe_candidate = 0;
  req->req.data = (void *) req;

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

  stat_bin_candidate(req);

  return 0;
}
