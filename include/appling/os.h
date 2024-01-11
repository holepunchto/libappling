#ifndef APPLING_OS_H
#define APPLING_OS_H

#include <stddef.h>

#include "arch.h"

#if defined(__linux__)
#include "linux.h"
#elif defined(_WIN32)
#include "win32.h"
#elif defined(__APPLE__)
#include "darwin.h"
#else
#error Unsupported operating system
#endif

#define APPLING_TARGET APPLING_OS "-" APPLING_ARCH

#define APPLING_PLATFORM_CURRENT "current" APPLING_PATH_SEPARATOR "by-arch" APPLING_PATH_SEPARATOR APPLING_TARGET

#define APPLING_PLATFORM_NEXT "next" APPLING_PATH_SEPARATOR "by-arch" APPLING_PATH_SEPARATOR APPLING_TARGET

#define APPLING_PLATFORM_CANDIDATES \
  { \
    APPLING_PLATFORM_CURRENT, \
    APPLING_PLATFORM_NEXT, \
    NULL, \
  };

#endif // APPLING_OS_H
