#ifndef HOLEPUNCH_OS_H
#define HOLEPUNCH_OS_H

#include "arch.h"

#if defined(__linux__)
#include "linux.h"
#elif defined(_WIN32)
#include "win.h"
#elif defined(__APPLE__)
#include "mac.h"
#else
#error Unsupported operating system
#endif

#define HOLEPUNCH_RUNTIME HOLEPUNCH_OS "-" HOLEPUNCH_ARCH

#define HOLEPUNCH_BIN "platform" HOLEPUNCH_PATH_SEPARATOR "bin"

#define HOLEPUNCH_BIN_NEXT "platform" HOLEPUNCH_PATH_SEPARATOR "bin-next"

#define HOLEPUNCH_BIN_FALLBACK "platform" HOLEPUNCH_PATH_SEPARATOR "stable" HOLEPUNCH_PATH_SEPARATOR "swap-0" HOLEPUNCH_PATH_SEPARATOR "bin" HOLEPUNCH_PATH_SEPARATOR HOLEPUNCH_RUNTIME

#define HOLEPUNCH_BIN_CANDIDATES \
  { \
    HOLEPUNCH_BIN, \
    HOLEPUNCH_BIN_NEXT, \
    HOLEPUNCH_BIN_FALLBACK, \
    NULL, \
  };

#endif // HOLEPUNCH_OS_H
