#include <uv.h>

static void
appling_lock__lock(int fd) {
  HANDLE handle = uv_get_osfhandle(fd);

  size_t length = SIZE_MAX;

  OVERLAPPED data = {
    .hEvent = 0,
  };

  LockFileEx(handle, LOCKFILE_EXCLUSIVE_LOCK, 0, length, length >> 32, &data);
}
