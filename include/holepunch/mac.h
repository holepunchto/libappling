#ifndef HOLEPUNCH_MAC_H
#define HOLEPUNCH_MAC_H

#include "posix.h"

#define HOLEPUNCH_OS "darwin"
#define HOLEPUNCH_OS_DARWIN

#define HOLEPUNCH_DIR "Library/Application Support/Holepunch"

#define HOLEPUNCH_PLATFORM_BUNDLE "../Resources/app/platform.mp3"

#define HOLEPUNCH_EXE_CANDIDATES \
  { \
    "Holepunch.app/Contents/MacOS/Holepunch", \
    NULL, \
  };

#endif // HOLEPUNCH_MAC_H
