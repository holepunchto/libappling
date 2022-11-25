#ifndef HOLEPUNCH_TEST_FIXTURES_APP_H
#define HOLEPUNCH_TEST_FIXTURES_APP_H

#if defined(HOLEPUNCH_OS_LINUX)
#define HOLEPUNCH_TEST_EXE "Example.AppDir/Example"
#elif defined(HOLEPUNCH_OS_WIN32)
#define HOLEPUNCH_TEST_EXE "Example.AppDir\\Example.exe"
#elif defined(HOLEPUNCH_OS_DARWIN)
#define HOLEPUNCH_TEST_EXE "Example.app/Contents/MacOS/Example"
#endif

#endif // HOLEPUNCH_TEST_FIXTURES_APP_H
