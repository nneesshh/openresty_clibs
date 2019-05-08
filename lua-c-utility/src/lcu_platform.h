#ifndef __LCU_PLATFORM_H__
#define __LCU_PLATFORM_H__

/*
   Supported platform:

     DARWIN   - Any Darwin system (macOS, iOS, watchOS, tvOS)
	 ANDROID  - Android platform
	 WIN32    - Win32 (Windows 2000/XP/Vista/7 and Windows Server 2003/2008)
	 WINRT    - WinRT (Windows Runtime)
	 CYGWIN   - Cygwin
	 LINUX    - Linux
	 FREEBSD  - FreeBSD
	 OPENBSD  - OpenBSD
	 SOLARIS  - Sun Solaris
	 AIX      - AIX
     UNIX     - Any UNIX BSD/SYSV system
*/

#define LCU_PLATFORM_VERSION 1.0.0.190314

// DARWIN
#if defined(__APPLE__) && (defined(__GNUC__) || defined(__xlC__) || defined(__xlc__))
#  include <TargetConditionals.h>
#  if defined(TARGET_OS_MAC) && TARGET_OS_MAC
#    define LCU_PLATFORM_DARWIN
#    ifdef __LP64__
#      define LCU_PLATFORM_DARWIN64
#    else
#      define LCU_PLATFORM_DARWIN32
#    endif
#  else
#    error "not support this Apple platform"
#  endif
// ANDROID
#elif defined(__ANDROID__) || defined(ANDROID)
#  define LCU_PLATFORM_ANDROID
#  define LCU_PLATFORM_LINUX
// Windows
#elif !defined(SAG_COM) && (!defined(WINAPI_FAMILY) || WINAPI_FAMILY==WINAPI_FAMILY_DESKTOP_APP) && (defined(WIN64) || defined(_WIN64) || defined(__WIN64__))
#  define LCU_PLATFORM_WIN32
#  define LCU_PLATFORM_WIN64
#elif !defined(SAG_COM) && (defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__))
#  if defined(WINAPI_FAMILY)
#    ifndef WINAPI_FAMILY_PC_APP
#      define WINAPI_FAMILY_PC_APP WINAPI_FAMILY_APP
#    endif
#    if defined(WINAPI_FAMILY_PHONE_APP) && WINAPI_FAMILY==WINAPI_FAMILY_PHONE_APP
#      define LCU_PLATFORM_WINRT
#    elif WINAPI_FAMILY==WINAPI_FAMILY_PC_APP
#      define LCU_PLATFORM_WINRT
#    else
#      define LCU_PLATFORM_WIN32
#    endif
#  else
#    define LCU_PLATFORM_WIN32
#  endif
//CYGWIN
#elif defined(__CYGWIN__)
#  define LCU_PLATFORM_CYGWIN
// sun os
#elif defined(__sun) || defined(sun)
#  define LCU_PLATFORM_SOLARIS
// LINUX
#elif defined(__linux__) || defined(__linux)
#  define LCU_PLATFORM_LINUX
// FREEBSD
#elif defined(__FreeBSD__) || defined(__DragonFly__) || defined(__FreeBSD_kernel__)
#  ifndef __FreeBSD_kernel__
#    define LCU_PLATFORM_FREEBSD
#  endif
#  define LCU_PLATFORM_FREEBSD_KERNEL
// OPENBSD
#elif defined(__OpenBSD__)
#  define LCU_PLATFORM_OPENBSD
// IBM AIX
#elif defined(_AIX)
#  define LCU_PLATFORM_AIX
#else
#  error "not support this OS"
#endif

#if defined(LCU_PLATFORM_WIN32) || defined(LCU_PLATFORM_WIN64) || defined(LCU_PLATFORM_WINRT)
#  define LCU_PLATFORM_WIN
#endif

#if defined(LCU_PLATFORM_WIN)
#  undef LCU_PLATFORM_UNIX
#elif !defined(LCU_PLATFORM_UNIX)
#  define LCU_PLATFORM_UNIX
#endif

#ifdef LCU_PLATFORM_DARWIN
#define LCU_PLATFORM_MAC
#endif
#ifdef LCU_PLATFORM_DARWIN32
#define LCU_PLATFORM_MAC32
#endif
#ifdef LCU_PLATFORM_DARWIN64
#define LCU_PLATFORM_MAC64
#endif

#ifdef __cplusplus
extern "C" {
#endif

enum {
	PLATFORM_WINDOWS = 1,
	PLATFORM_ANDROID,
	PLATFORM_LINUX,
	PLATFORM_DARWIN,

	PLATFORM_UNKNOWN = 9999,
};

int get_platform();
const char * get_platform_name();

#ifdef __cplusplus
}
#endif

#endif // __LCU_PLATFORM_H__