#include "lcu_platform.h"

int
get_platform()
{
#if defined(LCU_PLATFORM_WIN32) || defined(LCU_PLATFORM_WINRT)
	return PLATFORM_WINDOWS;
#elif defined(LCU_PLATFORM_ANDROID)
	return PLATFORM_ANDROID;
#elif defined(LCU_PLATFORM_LINUX)
	return PLATFORM_LINUX;
#elif defined(LCU_PLATFORM_DARWIN)
	return PLATFORM_DARWIN;
#else
	return PLATFORM_UNKNOWN;
#endif
}


const char *
get_platform_name()
{
#if defined(LCU_PLATFORM_WIN32) || defined(LCU_PLATFORM_WINRT)
	return "windows";
#elif defined(LCU_PLATFORM_ANDROID)
	return "android";
#elif defined(LCU_PLATFORM_LINUX)
	return "linux";
#elif defined(LCU_PLATFORM_DARWIN)
	return "apple";
#else
	return "unknown";
#endif
}
