#include <appling.h>
#include <assert.h>
#include <log.h>

#ifndef PLATFORM_DIR
#define PLATFORM_DIR "example/pear"
#endif

appling_platform_t platform = {
  .dkey = {0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa},
};

appling_app_t app = {
  .key = "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb",
};

int
main (int argc, char *argv[]) {
  int err;

  err = log_open("Appling", 0);
  assert(err == 0);

  err = appling_main(argc, argv, PLATFORM_DIR, &platform, &app);
  assert(err == 0);

  return 0;
}
