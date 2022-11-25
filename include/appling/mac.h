#ifndef APPLING_MAC_H
#define APPLING_MAC_H

#include "posix.h"

#define APPLING_OS "darwin"
#define APPLING_OS_DARWIN

#define APPLING_PLATFORM_DIR "Library/Application Support/Holepunch"

#define APPLING_PLATFORM_BUNDLE "../Resources/app/platform.mp3"

#define APPLING_EXE_CANDIDATES \
  { \
    "Holepunch.app/Contents/MacOS/Holepunch", \
    NULL, \
  };

#endif // APPLING_MAC_H
