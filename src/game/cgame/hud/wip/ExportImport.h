#ifndef IMPORTEXPORT_H
#define IMPORTEXPORT_H

#if defined(BUILDING_SHARED)
/* Building shared library. */
#  define EXPORTIMPORT EXPORT
#elif defined(USING_SHARED)
/* Using shared library. */
#  define EXPORTIMPORT IMPORT
#else
/* Building static library. */
#  define EXPORTIMPORT
#endif

#if defined(_MSC_VER)
#  define EXPORT __declspec(dllexport)
#  define IMPORT __declspec(dllimport)
#elif defined(__GNUC__) || defined(__clang__)
#  define EXPORT __attribute__((visibility("default")))
#  define IMPORT
#else
#  error Unsupported compiler.
#endif

#if defined(_MSC_VER)
#  define QDECL __cdecl
#elif defined(__clang__)
#  define QDECL __attribute__((cdecl))
#elif defined(__GNUC__)
#  if defined(__x86_64__)
#    define QDECL // warning: 'cdecl' attribute ignored [-Wattributes]
#  elif defined(__i386__)
#    define QDECL __attribute__((cdecl))
#  else
#    error Unsupported architecture.
#  endif
#else
#  error Unsupported compiler.
#endif

#endif // IMPORTEXPORT_H
