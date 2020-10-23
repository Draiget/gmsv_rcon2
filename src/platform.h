#pragma once
// ReSharper disable IdentifierTypo
// ReSharper disable CppInconsistentNaming

#ifdef _PLATFORM_LINUX
	#ifndef _GNU_SOURCE
	#define _GNU_SOURCE
	#endif
	#ifndef __USE_GNU
	#define __USE_GNU
	#endif
	#ifndef LINUX
	#define LINUX
	#endif
	#ifndef _LINUX
	#define _LINUX
	#endif
	#ifndef __GCC__
	#define __GCC__
	#endif
	#ifndef _GNUC
	#define _GNUC
	#endif
	#ifndef _POSIX
	#define _POSIX
	#endif
	#ifndef _M_IX86
	#define _M_IX86
	#endif
	#define GNUC defined(_GNUC)
	#define POSIX defined(_POSIX)

	#ifdef GMSV_RCON2_CODE_LINKAGE
	#pragma comment(lib, "libtier0.so")
	#pragma comment(lib, "tier1.a")
	#pragma comment(lib, "tier3.a")
	#pragma comment(lib, "tier2.a")
	#pragma comment(lib, "libvstdlib.so")
	#endif

	#undef _MSC_VER
	#undef _WIN32
	#undef _WIN64
#else
	#ifndef _WIN32
	#define _WIN32
	#endif
	#ifndef _X86_
	#define _X86_
	#endif
	#include <windows.h>
	#pragma comment(lib, "legacy_stdio_definitions.lib")
#endif

#ifdef _PLATFORM_UKNOWN
	#error "Unsupported platform!"
#endif

#define GMSV_RCON2_PRINT_PREFIX "RCON2"

// ReSharper restore IdentifierTypo
// ReSharper restore CppInconsistentNaming
