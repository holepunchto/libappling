#include <archive.h>
#include <archive_entry.h>
#include <limits.h>
#include <path.h>
#include <stdlib.h>
#include <string.h>
#include <uv.h>

#include "../include/holepunch.h"

static inline int
copy (struct archive *from, struct archive *to) {
  int r;
  const void *buf;
  size_t size;
  int64_t offset;

  for (;;) {
    r = archive_read_data_block(from, &buf, &size, &offset);

    if (r == ARCHIVE_EOF) return ARCHIVE_OK;
    if (r != ARCHIVE_OK) return r;

    r = archive_write_data_block(to, buf, size, offset);

    if (r != ARCHIVE_OK) return r;
  }
}

static inline void
extract (holepunch_extract_t *req, const char *archive, const char *dest) {
  struct archive *reader;
  struct archive *writer;

  reader = archive_read_new();
  writer = archive_write_disk_new();

  archive_read_support_format_tar(reader);

  int r = archive_read_open_filename(reader, archive, 10240);
  if (r) {
    req->status = -r;
    goto done;
  }

  char path[PATH_MAX];

  struct archive_entry *entry;

  for (;;) {
    r = archive_read_next_header(reader, &entry);

    if (r == ARCHIVE_EOF) break;
    if (r != ARCHIVE_OK) {
      req->status = r;
      goto done;
    }

    size_t path_len = PATH_MAX;

    path_join(
      (const char *[]){dest, archive_entry_pathname(entry), NULL},
      path,
      &path_len,
      path_separator_system
    );

    archive_entry_set_pathname(entry, path);

    r = archive_write_header(writer, entry);
    if (r != ARCHIVE_OK) {
      req->status = r;
      goto done;
    }

    r = copy(reader, writer);
    if (r != ARCHIVE_OK) {
      req->status = r;
      goto done;
    }

    r = archive_write_finish_entry(writer);
    if (r != ARCHIVE_OK) {
      req->status = r;
      goto done;
    }
  }

done:
  archive_read_free(reader);
  archive_write_free(writer);
}

static void
on_work (uv_work_t *handle) {
  holepunch_extract_t *req = (holepunch_extract_t *) handle->data;

  extract(req, req->archive, req->dest);
}

static void
on_after_work (uv_work_t *handle, int status) {
  holepunch_extract_t *req = (holepunch_extract_t *) handle->data;

  if (req->status < 0) status = req->status;

  free(req->archive);
  free(req->dest);

  if (req->cb) req->cb(req, status);
}

int
holepunch_extract (uv_loop_t *loop, holepunch_extract_t *req, const char *archive, const char *dest, holepunch_extract_cb cb) {
  req->loop = loop;
  req->cb = cb;
  req->archive = strdup(archive);
  req->dest = strdup(dest);
  req->status = 0;
  req->req.data = (void *) req;

  return uv_queue_work(loop, &req->req, on_work, on_after_work);
}
