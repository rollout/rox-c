#pragma once

#ifdef DOXYGEN_SHOULD_SKIP_THIS
    #define DLL_PUBLIC
#else

// Generic helper definitions for shared library support.
// (see https://gcc.gnu.org/wiki/Visibility/)
    #if defined _WIN32 || defined __CYGWIN__
        #define ROX_HELPER_DLL_IMPORT __declspec(dllimport)
        #define ROX_HELPER_DLL_EXPORT __declspec(dllexport)
        #define ROX_HELPER_DLL_LOCAL
    #else
        #if __GNUC__ >= 4
            #define ROX_HELPER_DLL_IMPORT __attribute__ ((visibility ("default")))
            #define ROX_HELPER_DLL_EXPORT __attribute__ ((visibility ("default")))
            #define ROX_HELPER_DLL_LOCAL  __attribute__ ((visibility ("hidden")))
        #else
            #define ROX_HELPER_DLL_IMPORT
            #define ROX_HELPER_DLL_EXPORT
            #define ROX_HELPER_DLL_LOCAL
        #endif // __GNUC__ >= 4
    #endif // defined _WIN32 || defined __CYGWIN__

// Now we use the generic helper definitions above to define ROX_API and ROX_LOCAL.
// ROX_API is used for the public API symbols. It either DLL imports or DLL exports (or does nothing for static build)
// ROX_LOCAL is used for non-api symbols.

    #ifdef ROX_DLL // defined if ROX is compiled as a DLL
        #ifdef ROX_DLL_EXPORTS // defined if we are building the ROX DLL (instead of using it)
            #define ROX_API ROX_HELPER_DLL_EXPORT
        #else
            #define ROX_API ROX_HELPER_DLL_IMPORT
        #endif // ROX_DLL_EXPORTS
        #define ROX_LOCAL ROX_HELPER_DLL_LOCAL
    #else // ROX_DLL is not defined: this means ROX is a static lib.
        #define ROX_API
        #define ROX_LOCAL
    #endif // ROX_DLL

#endif // DOXYGEN_SHOULD_SKIP_THIS

#ifdef __cplusplus
extern "C"
{
#endif

// TODO: put C functions exports here

#ifdef __cplusplus
} // extern "C"
#endif

#ifdef __cplusplus

// TODO: put API class(es) declaration here

#endif