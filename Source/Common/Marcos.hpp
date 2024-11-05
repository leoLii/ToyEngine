
#if defined(_WIN64)
#define ASRCH_OS_WINDOWS
#elif defined(__linux__)
#define ASRCH_OS_LINUX
#elif defines(__APPLE__)
#define ASRCH_OS_MACOS
#else
#error "Unsupported target platform"
#endif

#define REVERSE_DEPTH

#define NOMINMAX

#define NO_COPY(typeName) \
    typeName(const typeName&) = delete; \
    typeName& operator=(const typeName&) = delete;

#define NO_MOVE(typeName) \
    typeName(const typeName&&) = delete; \
    typeName& operator=(const typeName&&) = delete;