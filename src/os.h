#if defined(__FreeBSD__) || defined(__FreeBSD)
#define ROX_FREE_BSD
#elif defined(__linux) || defined(__linux__) || defined(linux)
#define ROX_LINUX
#elif defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
#define ROX_WINDOWS
#elif defined(__APPLE__)
#define ROX_APPLE
#endif
