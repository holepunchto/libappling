#include <assert.h>
#include <path.h>
#include <stdlib.h>
#include <string.h>
#include <uv.h>

#include "../include/appling.h"

#if defined(APPLING_OS_DARWIN)
#include "darwin/lock.h"
#elif defined(APPLING_OS_LIUX)
#include "linux/lock.h"
#elif defined(APPLING_OS_WIN32)
#include "win32/lock.h"
#endif

static void
appling_lock__on_close(uv_handle_t *handle) {
  int err;

  appling_lock_t *req = (appling_lock_t *) handle->data;

  err = uv_thread_join(&req->thread);
  assert(err == 0);

  req->cb(req, req->status);
}

static void
appling_lock__on_signal(uv_async_t *handle) {
  uv_close((uv_handle_t *) handle, appling_lock__on_close);
}

static void
appling_lock__on_lock(void *data) {
  int err;

  appling_lock_t *req = (appling_lock_t *) data;

  appling_path_t path;
  size_t path_len = sizeof(appling_path_t);

  err = path_join(
    (const char *[]) {req->dir, "lock", NULL},
    path,
    &path_len,
    path_behavior_system
  );
  assert(err == 0);

  uv_fs_t fs;

  appling_path_t dir;

  strcpy(dir, req->dir);

  while (true) {
    err = uv_fs_mkdir(NULL, &fs, dir, 0775, NULL);

    uv_fs_req_cleanup(&fs);

    switch (err) {
    case UV_EACCES:
    case UV_ENOSPC:
    case UV_ENOTDIR:
    case UV_EPERM:
    default:
      req->status = err; // Propagate
      goto done;

    case UV_EEXIST:
    case 0: {
      size_t len = strlen(dir);

      if (len == strlen(req->dir)) goto open;

      dir[len] = '/';

      continue;
    }

    case UV_ENOENT: {
      size_t dirname;
      err = path_dirname(dir, &dirname, path_behavior_system);
      assert(err == 0);

      if (dirname == strlen(req->dir)) goto open;

      dir[dirname - 1] = '\0';

      continue;
    }
    }
  }

open:
  err = uv_fs_open(NULL, &fs, path, UV_FS_O_RDWR | UV_FS_O_CREAT, 0666, NULL);
  if (err < 0) {
    req->status = err; // Propagate
    goto done;
  }

  req->file = err;
  req->status = 0;

  appling_lock__lock(req->file);

done:
  err = uv_async_send(&req->signal);
  assert(err == 0);
}

int
appling_lock(uv_loop_t *loop, appling_lock_t *req, const char *dir, appling_lock_cb cb) {
  int err;

  req->loop = loop;
  req->cb = cb;
  req->file = -1;
  req->signal.data = (void *) req;

  if (dir && path_is_absolute(dir, path_behavior_system)) strcpy(req->dir, dir);
  else if (dir) {
    appling_path_t cwd;
    size_t path_len = sizeof(appling_path_t);

    err = uv_cwd(cwd, &path_len);
    if (err < 0) return err;

    path_len = sizeof(appling_path_t);

    err = path_join(
      (const char *[]) {cwd, dir, NULL},
      req->dir,
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
      req->dir,
      &path_len,
      path_behavior_system
    );
    assert(err == 0);
  }

  err = uv_async_init(req->loop, &req->signal, appling_lock__on_signal);
  assert(err == 0);

  err = uv_thread_create(&req->thread, appling_lock__on_lock, req);
  assert(err == 0);

  return 0;
}
