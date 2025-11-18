#include <fcntl.h>

static void
appling_lock__lock(int fd) {
  struct flock data = {
    .l_pid = 0,
    .l_type = F_WRLCK,
    .l_whence = SEEK_SET,
  };

  fcntl(fd, F_OFD_SETLKW, &data);
}
