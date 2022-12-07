#ifndef APPLING_TEST_FIXTURES_APP_H
#define APPLING_TEST_FIXTURES_APP_H

#if defined(APPLING_OS_DARWIN)
#define APPLING_TEST_EXE "Example.app/Contents/MacOS/Example"
#elif defined(APPLING_OS_LINUX)
#define APPLING_TEST_EXE "Example.AppDir/usr/bin/example"
#elif defined(APPLING_OS_WIN32)
#define APPLING_TEST_EXE "Example\\Example.exe"
#endif

#endif // APPLING_TEST_FIXTURES_APP_H
