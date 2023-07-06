#include <hex.h>
#include <log.h>
#include <path.h>
#include <stdlib.h>
#include <string.h>
#include <utf.h>
#include <uv.h>

#include "../include/appling.h"

static void
on_symlink (fs_symlink_t *fs_req, int status) {
  appling_bootstrap_t *req = (appling_bootstrap_t *) fs_req->data;

  if (status >= 0) {
    if (req->cb) req->cb(req, 0);
  } else {
    if (req->cb) req->cb(req, status);
  }
}

static void
symlink_current (appling_bootstrap_t *req) {
  char key[65];
  size_t key_len = 65;

  hex_encode(req->key, APPLING_KEY_LEN, (utf8_t *) key, &key_len);

  appling_path_t target;
  size_t path_len = sizeof(appling_path_t);

  path_join(
    (const char *[]){"by-dkey", key, "0", NULL},
    target,
    &path_len,
    path_behavior_system
  );

  appling_path_t link;
  path_len = sizeof(appling_path_t);

  path_join(
    (const char *[]){req->dir, "current", NULL},
    link,
    &path_len,
    path_behavior_system
  );

  log_debug("appling_bootstrap() linking platform at %s", target);

  fs_symlink(req->loop, &req->symlink, target, link, UV_FS_SYMLINK_DIR, on_symlink);
}

static void
on_merge (fs_merge_t *fs_req, int status) {
  appling_bootstrap_t *req = (appling_bootstrap_t *) fs_req->data;

  if (status >= 0) {
    symlink_current(req);
  } else {
    if (req->cb) req->cb(req, status);
  }
}

static void
on_mkdir (fs_mkdir_t *fs_req, int status) {
  appling_bootstrap_t *req = (appling_bootstrap_t *) fs_req->data;

  if (status >= 0) {
    char key[65];
    size_t key_len = 65;

    hex_encode(req->key, APPLING_KEY_LEN, (utf8_t *) key, &key_len);

    appling_path_t base;
    size_t path_len = sizeof(appling_path_t);

    path_join(
      (const char *[]){req->dir, "by-dkey", key, "0", "corestore", NULL},
      base,
      &path_len,
      path_behavior_system
    );

    appling_path_t onto;
    path_len = sizeof(appling_path_t);

    path_join(
      (const char *[]){req->dir, "corestores", "platform", NULL},
      onto,
      &path_len,
      path_behavior_system
    );

    log_debug("appling_bootstrap() merging corestores at %s", onto);

    fs_merge(req->loop, &req->merge, base, onto, true, on_merge);
  } else {
    if (req->cb) req->cb(req, status);
  }
}

static void
on_rmdir_tmp (fs_rmdir_t *fs_req, int status) {
  appling_bootstrap_t *req = (appling_bootstrap_t *) fs_req->data;

  status = req->status;

  if (status >= 0) {
    appling_path_t path;
    size_t path_len = sizeof(appling_path_t);

    path_join(
      (const char *[]){req->dir, "corestores", NULL},
      path,
      &path_len,
      path_behavior_system
    );

    fs_mkdir(req->loop, &req->mkdir, path, 0777, true, on_mkdir);
  } else {
    if (req->cb) req->cb(req, status);
  }
}

static void
discard_tmp (appling_bootstrap_t *req) {
  char key[65];
  size_t key_len = 65;

  hex_encode(req->key, APPLING_KEY_LEN, (utf8_t *) key, &key_len);

  appling_path_t tmp;
  size_t path_len = sizeof(appling_path_t);

  path_join(
    (const char *[]){req->dir, "by-dkey", key, "tmp", NULL},
    tmp,
    &path_len,
    path_behavior_system
  );

  fs_rmdir(req->loop, &req->rmdir, tmp, true, on_rmdir_tmp);
}

static void
on_rename (fs_rename_t *fs_req, int status) {
  appling_bootstrap_t *req = (appling_bootstrap_t *) fs_req->data;

  if (status >= 0) {
    req->status = 0;
  } else {
    req->status = status; // Propagate
  }

  discard_tmp(req);
}

static inline void
rename_platform (appling_bootstrap_t *req) {
  char key[65];
  size_t key_len = 65;

  hex_encode(req->key, APPLING_KEY_LEN, (utf8_t *) key, &key_len);

  appling_path_t to;
  size_t path_len = sizeof(appling_path_t);

  path_join(
    (const char *[]){req->dir, "by-dkey", key, "0", NULL},
    to,
    &path_len,
    path_behavior_system
  );

  appling_path_t from;
  path_len = sizeof(appling_path_t);

  path_join(
    (const char *[]){req->dir, "by-dkey", key, "tmp", NULL},
    from,
    &path_len,
    path_behavior_system
  );

  log_debug("appling_bootstrap() renaming platform at %s", to);

  fs_rename(req->loop, &req->rename, from, to, on_rename);
}

static void
on_swap (fs_swap_t *fs_req, int status) {
  appling_bootstrap_t *req = (appling_bootstrap_t *) fs_req->data;

  if (status >= 0) {
    discard_tmp(req);
  } else {
    req->status = status; // Propagate

    rename_platform(req);
  }
}

static inline void
swap_platform (appling_bootstrap_t *req) {
  char key[65];
  size_t key_len = 65;

  hex_encode(req->key, APPLING_KEY_LEN, (utf8_t *) key, &key_len);

  appling_path_t to;
  size_t path_len = sizeof(appling_path_t);

  path_join(
    (const char *[]){req->dir, "by-dkey", key, "0", NULL},
    to,
    &path_len,
    path_behavior_system
  );

  appling_path_t from;
  path_len = sizeof(appling_path_t);

  path_join(
    (const char *[]){req->dir, "by-dkey", key, "tmp", NULL},
    from,
    &path_len,
    path_behavior_system
  );

  log_debug("appling_bootstrap() swapping platform at %s", to);

  fs_swap(req->loop, &req->swap, from, to, on_swap);
}

static void
on_extract (appling_extract_t *extract, int status) {
  appling_bootstrap_t *req = (appling_bootstrap_t *) extract->data;

  if (status >= 0) {
    swap_platform(req);
  } else {
    req->status = status; // Propagate

    discard_tmp(req);
  }
}

static inline void
extract_platform (appling_bootstrap_t *req) {
  char key[65];
  size_t key_len = 65;

  hex_encode(req->key, APPLING_KEY_LEN, (utf8_t *) key, &key_len);

  appling_path_t archive;
  size_t path_len = sizeof(appling_path_t);

  path_join(
    (const char *[]){req->exe_dir, appling_platform_bundle, NULL},
    archive,
    &path_len,
    path_behavior_system
  );

  appling_path_t dest;
  path_len = sizeof(appling_path_t);

  path_join(
    (const char *[]){req->dir, "by-dkey", key, "tmp", NULL},
    dest,
    &path_len,
    path_behavior_system
  );

  log_debug("appling_bootstrap() extracting platform archive to %s", dest);

  appling_extract(req->loop, &req->extract, archive, dest, on_extract);
}

static void
on_rmdir_tmp_maybe (fs_rmdir_t *fs_req, int status) {
  appling_bootstrap_t *req = (appling_bootstrap_t *) fs_req->data;

  extract_platform(req);
}

static inline int
discard_tmp_maybe (appling_bootstrap_t *req) {
  char key[65];
  size_t key_len = 65;

  hex_encode(req->key, APPLING_KEY_LEN, (utf8_t *) key, &key_len);

  appling_path_t tmp;
  size_t path_len = sizeof(appling_path_t);

  path_join(
    (const char *[]){req->dir, "by-dkey", key, "tmp", NULL},
    tmp,
    &path_len,
    path_behavior_system
  );

  return fs_rmdir(req->loop, &req->rmdir, tmp, true, on_rmdir_tmp_maybe);
}

int
appling_bootstrap (uv_loop_t *loop, appling_bootstrap_t *req, const appling_key_t key, const char *exe, const char *dir, appling_bootstrap_cb cb) {
  req->loop = loop;
  req->cb = cb;
  req->status = 0;
  req->swap.data = (void *) req;
  req->rename.data = (void *) req;
  req->rmdir.data = (void *) req;
  req->mkdir.data = (void *) req;
  req->merge.data = (void *) req;
  req->symlink.data = (void *) req;
  req->extract.data = (void *) req;
  req->resolve.data = (void *) req;

  memcpy(req->key, key, sizeof(appling_key_t));

  appling_path_t path;
  size_t path_len;

  if (exe) strcpy(path, exe);
  else {
    path_len = sizeof(appling_path_t);
    uv_exepath(path, &path_len);
  }

  path_len = sizeof(appling_path_t);

  path_join(
    (const char *[]){path, "..", NULL},
    req->exe_dir,
    &path_len,
    path_behavior_system
  );

  if (dir) strcpy(req->dir, dir);
  else {
    appling_path_t homedir;
    path_len = sizeof(appling_path_t);

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

  return discard_tmp_maybe(req);
}
