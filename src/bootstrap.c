#include <path.h>
#include <string.h>
#include <uv.h>

#include "../include/appling.h"

int
appling_bootstrap (uv_loop_t *loop, appling_bootstrap_t *req, const appling_dkey_t dkey, const char *exe, const char *dir, appling_bootstrap_cb cb) {
  req->loop = loop;
  req->cb = cb;
  req->status = 0;

  memcpy(req->dkey, dkey, sizeof(appling_dkey_t));

  strcpy(req->exe, exe);

  if (dir && path_is_absolute(dir, path_behavior_system)) strcpy(req->dir, dir);
  else if (dir) {
    appling_path_t cwd;
    size_t path_len = sizeof(appling_path_t);

    int err = uv_cwd(cwd, &path_len);
    if (err < 0) return err;

    path_len = sizeof(appling_path_t);

    path_join(
      (const char *[]){cwd, dir, NULL},
      req->dir,
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
      (const char *[]){homedir, appling_platform_dir, NULL},
      req->dir,
      &path_len,
      path_behavior_system
    );
  }

  return 0;
}
