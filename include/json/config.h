// Copyright 2007-2010 Baptiste Lepilleur
// Distributed under MIT license, or public domain if desired and
// recognized in your jurisdiction.
// See file LICENSE for detail or copy at http://jsoncpp.sourceforge.net/LICENSE

#ifndef JSONCPP_CONFIG_H_INCLUDED
#define JSONCPP_CONFIG_H_INCLUDED

/// If defined, indicates that Json use exception to report invalid type manipulation
/// instead of C assert macro.
#define JSONCPP_USE_EXCEPTION 1

/// If defined, indicates that the source file is amalgated
/// to prevent private header inclusion.
/// Remarks: it is automatically defined in the generated amalgated header.
// #define JSONCPP_IS_AMALGAMATION

/// If defined, all asserts will be enabled. Else, they will have no effect
/// Remarks: defining this macro enables the functionality of: JSONCPP_ASSERT_UNREACHABLE, JSONCPP_ASSERT,
/// JSONCPP_FAIL_MESSAGE, and JSONCPP_ASSERT_MESSAGE
// # define JSONCPP_ENABLE_ASSERTS 1

#ifdef JSONCPP_IN_CPPTL
#include <cpptl/config.h>
#ifndef JSONCPP_USE_CPPTL
#define JSONCPP_USE_CPPTL 1
#endif
#endif

#ifdef JSONCPP_IN_CPPTL
#define JSONCPP_API CPPTL_API
#elif defined(JSONCPP_DLL_BUILD)
#define JSONCPP_API __declspec(dllexport)
#elif defined(JSONCPP_DLL)
#define JSONCPP_API __declspec(dllimport)
#else
#define JSONCPP_API
#endif

// If JSONCPP_NO_INT64 is defined, then Json only support C++ "int" type for integer
// Storages, and 64 bits integer support is disabled.
// #define JSONCPP_NO_INT64 1

#if defined(_MSC_VER) && _MSC_VER <= 1200 // MSVC 6
// Microsoft Visual Studio 6 only support conversion from __int64 to double
// (no conversion from unsigned __int64).
#define JSONCPP_USE_INT64_DOUBLE_CONVERSION 1
#endif // if defined(_MSC_VER)  &&  _MSC_VER < 1200 // MSVC 6

#if defined(_MSC_VER) && _MSC_VER >= 1500 // MSVC 2008
/// Indicates that the following function is deprecated.
#define JSONCPP_DEPRECATED(message) __declspec(deprecated(message))
#endif

#if !defined(JSONCPP_DEPRECATED)
#define JSONCPP_DEPRECATED(message)
#endif // if !defined(JSONCPP_DEPRECATED)

namespace Json {
    typedef int Int;
    typedef unsigned int UInt;
#if defined(JSONCPP_NO_INT64)
    typedef int LargestInt;
    typedef unsigned int LargestUInt;
#undef JSONCPP_HAS_INT64
#else                 // if defined(JSONCPP_NO_INT64)
    // For Microsoft Visual use specific types as long long is not supported
#if defined(_MSC_VER) // Microsoft Visual Studio
    typedef __int64 Int64;
    typedef unsigned __int64 UInt64;
#else                 // if defined(_MSC_VER) // Other platforms, use long long
    typedef long long int Int64;
    typedef unsigned long long int UInt64;
#endif                // if defined(_MSC_VER)
    typedef Int64 LargestInt;
    typedef UInt64 LargestUInt;
#define JSONCPP_HAS_INT64
#endif // if defined(JSONCPP_NO_INT64)
} // end namespace Json

#endif // JSONCPP_CONFIG_H_INCLUDED
