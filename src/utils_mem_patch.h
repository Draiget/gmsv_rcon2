#pragma once
#include "platform.h"

#ifdef _PLATFORM_LINUX
#include <sys/types.h>
#else
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
#endif
#ifdef _PLATFORM_UKNOWN
#error "Unsupported platform!"
#endif

// ReSharper disable once IdentifierTypo
namespace zontwelg
{
	struct shared_module
	{
		void* base_address;
		size_t base_size;
	};

	class memory_patcher
	{
	public:
		memory_patcher() = default;

		static shared_module* find_module(const char* name, bool* success);
		static memory_patcher* get_instance();

#ifndef _PLATFORM_LINUX
		static void free_process_handle(HANDLE handle);
#endif

	private:
		static memory_patcher* s_instance_;
	};
}
