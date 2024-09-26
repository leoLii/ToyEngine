
#if defined(_WIN64)
#define ASRCH_OS_WINDOWS
#elif defined(__linux__)
#define ASRCH_OS_LINUX
#elif defines(__APPLE__)
#define ASRCH_OS_MACOS
#else
#error "Unsupported target platform"
#endif


#define NOMINMAX