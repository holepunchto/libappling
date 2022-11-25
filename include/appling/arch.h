#ifndef APPLING_ARCH_H
#define APPLING_ARCH_H

#if defined(__aarch64__) || defined(_M_ARM64)
#define APPLING_ARCH "arm64"
#define APPLING_ARCH_ARM64
#elif defined(i386) || defined(__i386__) || defined(__i386) || defined(_M_IX86)
#define APPLING_ARCH "ia32"
#define APPLING_ARCH_IA32
#elif defined(__x86_64__) || defined(_M_X64)
#define APPLING_ARCH "x64"
#define APPLING_ARCH_X64
#else
#error Unsupported architecture
#endif

#endif // APPLING_ARCH_H
