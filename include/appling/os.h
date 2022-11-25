#ifndef APPLING_OS_H
#define APPLING_OS_H

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

#define APPLING_RUNTIME APPLING_OS "-" APPLING_ARCH

#define APPLING_BIN "platform" APPLING_PATH_SEPARATOR "bin"

#define APPLING_BIN_NEXT "platform" APPLING_PATH_SEPARATOR "bin-next"

#define APPLING_BIN_FALLBACK "platform" APPLING_PATH_SEPARATOR "stable" APPLING_PATH_SEPARATOR "swap-0" APPLING_PATH_SEPARATOR "bin" APPLING_PATH_SEPARATOR APPLING_RUNTIME

#define APPLING_BIN_CANDIDATES \
  { \
    APPLING_BIN, \
    APPLING_BIN_NEXT, \
    APPLING_BIN_FALLBACK, \
    NULL, \
  };

#endif // APPLING_OS_H
