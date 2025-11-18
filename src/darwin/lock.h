#include <fcntl.h>

static void
appling_lock__lock(int fd) {
  flock(fd, LOCK_EX);
}
