#ifndef APPLING_TEST_FIXTURES_APP_H
#define APPLING_TEST_FIXTURES_APP_H

#if defined(APPLING_OS_LINUX)
#define APPLING_TEST_EXE "Example.AppDir/Example"
#elif defined(APPLING_OS_WIN32)
#define APPLING_TEST_EXE "Example.AppDir\\Example.exe"
#elif defined(APPLING_OS_DARWIN)
#define APPLING_TEST_EXE "Example.app/Contents/MacOS/Example"
#endif

#endif // APPLING_TEST_FIXTURES_APP_H
