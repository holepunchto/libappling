#ifndef HOLEPUNCH_TEST_FIXTURES_APP_H
#define HOLEPUNCH_TEST_FIXTURES_APP_H

#if defined(__linux__)
#define HOLEPUNCH_TEST_EXE "Example.AppDir/Example"
#elif defined(_WIN32)
#define HOLEPUNCH_TEST_EXE "Example.AppDir\\Example.exe"
#elif defined(__APPLE__)
#define HOLEPUNCH_TEST_EXE "Example.app/Contents/MacOS/Example"
#endif

#endif // HOLEPUNCH_TEST_FIXTURES_APP_H
