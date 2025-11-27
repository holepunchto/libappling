#include <stdio.h>
#include <string.h>

int
main(int argc, char *argv[]) {
  for (int i = 0; i < argc; i++) {
    printf("%d=%s\n", i, argv[i]);
  }

  if (
    argc >= 5 &&
    strcmp(argv[1], "run") == 0 &&
    strcmp(argv[2], "--trusted") == 0 &&
    strcmp(argv[3], "--preflight") == 0 &&
    strcmp(argv[4], "--json") == 0
  ) {
    for (int i = 0, n = 10; i < n; i++) {
      printf(
        "{\"cmd\":\"run\",\"tag\":\"stats\",\"data\":{\"peers\":%d,\"download\":{\"bytes\":%d,\"blocks\":%d,\"speed\":%.17g,\"progress\":%.17g},\"upload\":{\"bytes\":%d,\"blocks\":%d,\"speed\":%.17g}}}\n",
        i,
        i * 1024,
        i * 10,
        123.0,
        1.0 / n * (i + 1),
        i * 512,
        i * 5,
        123.0
      );
    }

    return 0;
  }
}
