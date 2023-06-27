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

#define APPLING_BIN "current" APPLING_PATH_SEPARATOR "by-arch" APPLING_PATH_SEPARATOR APPLING_RUNTIME APPLING_PATH_SEPARATOR "bin"

#define APPLING_BIN_NEXT "current-next" APPLING_PATH_SEPARATOR "by-arch" APPLING_PATH_SEPARATOR APPLING_RUNTIME APPLING_PATH_SEPARATOR "bin"

#define APPLING_BIN_CANDIDATES \
  { \
    APPLING_BIN, \
    APPLING_BIN_NEXT, \
    NULL, \
  };

#endif // APPLING_OS_H
