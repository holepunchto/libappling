#ifndef HOLEPUNCH_ARCH_H
#define HOLEPUNCH_ARCH_H

#if defined(__aarch64__) || defined(_M_ARM64)
#define HOLEPUNCH_ARCH "arm64"
#elif defined(i386) || defined(__i386__) || defined(__i386) || defined(_M_IX86)
#define HOLEPUNCH_ARCH "ia32"
#elif defined(__x86_64__) || defined(_M_X64)
#define HOLEPUNCH_ARCH "x64"
#else
#error Unsupported architecture
#endif

#endif // HOLEPUNCH_ARCH_H
