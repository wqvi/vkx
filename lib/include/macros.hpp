#pragma once

#define DO_PRAGMA_(x) _Pragma(#x)
#define DO_PRAGMA(x) DO_PRAGMA_(x)

#ifdef _MSC_VER
#define PADDING_WARNING 4324

#define DISABLE_WARNING(x) _Pragma("warning(push)") \
    DO_PRAGMA(warning(disable : x))

#define RENABLE_WARNING _Pragma("warning(pop)")
#else
#define PADDING_WARNING
#define DISABLE_WARNING(x)
#define RENABLE_WARNING
#endif