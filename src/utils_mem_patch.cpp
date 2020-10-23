#include "utils_mem_patch.h"
#include "platform.h"
#include "validation.h"
#include <cstdio>
#include <cstring>
#include "utils_ssdk.h"
using namespace zontwelg;

#ifdef _PLATFORM_LINUX
#include "platform.h"
#include <sys/types.h>
#include <sys/stat.h> 
#include <link.h>
#include <dlfcn.h>
#else
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
#include <TlHelp32.h>
#include <windows.h>
#endif

memory_patcher* memory_patcher::s_instance_ = nullptr;

#ifdef _PLATFORM_LINUX
struct lookup_module
{
	const char* name;
	dl_phdr_info* result;
};

int check_module(struct dl_phdr_info* info, size_t size, void* data) {
	const auto lookup = static_cast<lookup_module*>(data);
#ifdef MODULE_LOOKUP_DEBUG
	Msg("[%s] Module lookup - '%s'\n", GMSV_RCON2_PRINT_PREFIX, info->dlpi_name);
#endif
	if (strstr(info->dlpi_name, lookup->name) != nullptr) {
		lookup->result = info;
		return 1;
	}

	return 0;
}
#endif

shared_module* memory_patcher::find_module(const char* name, bool* success) {
	const auto module_info = new shared_module();

#ifdef _PLATFORM_LINUX
	lookup_module lookup{};
	lookup.name = name;
	dl_iterate_phdr(check_module, &lookup);

	if (!is_valid_ptr(lookup.result)) {
		Msg("[%s] Module '%s' not found!\n", GMSV_RCON2_PRINT_PREFIX, name);
		*success = false;
		return nullptr;
	}

	struct stat dl_stat {};
	Dl_info dl_info;
	if (dladdr(reinterpret_cast<void*>(lookup.result->dlpi_addr), &dl_info) == 0) {
		Msg("[%s] No shared objects contains '%s' address! Error: %s\n", GMSV_RCON2_PRINT_PREFIX, name, dlerror());
		*success = false;
		return nullptr;
	}

	if (!dl_info.dli_fbase || !dl_info.dli_fname) {
		return nullptr;
	}

	module_info->base_address = static_cast<unsigned char*>(dl_info.dli_fbase);

	if (stat(dl_info.dli_fname, &dl_stat) != 0) {
		return nullptr;
	}

	module_info->base_size = dl_stat.st_size;
#ifdef MODULE_LOOKUP_DEBUG
	Msg("[%s] Module '%s' [base=%x, size=%d]\n", GMSV_RCON2_PRINT_PREFIX, name, reinterpret_cast<unsigned int>(module_info->base_address), module_info->base_size);
#endif

	*success = true;
	return module_info;

#else
	tagMODULEENTRY32 module_entry{};
	auto h_all_modules = INVALID_HANDLE_VALUE;
	if ((h_all_modules = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, 0)) == INVALID_HANDLE_VALUE) {
		*success = false;
		return nullptr;
	}

	module_entry.dwSize = sizeof(tagMODULEENTRY32);
	if (Module32First(h_all_modules, &module_entry)) {
		while (true) {
			module_entry.dwSize = sizeof(tagMODULEENTRY32);
#ifdef MODULE_LOOKUP_DEBUG
			Msg("[%s] Module lookup - '%s'\n", GMSV_RCON2_PRINT_PREFIX, module_entry.szModule);
#endif
			if (!_stricmp(module_entry.szModule, name)) {
				free_process_handle(h_all_modules);
				module_info->base_address = module_entry.modBaseAddr;
				module_info->base_size = module_entry.modBaseSize;

				*success = true;
				return module_info;
			}

			if (!Module32Next(h_all_modules, &module_entry)) {
				break;
			}
		}
	}

	free_process_handle(h_all_modules);
	*success = false;
	return nullptr;
#endif
}

memory_patcher* memory_patcher::get_instance() {
	if (!is_valid_ptr(s_instance_)) {
		s_instance_ = new memory_patcher();
	}

	return s_instance_;
}

#ifndef _PLATFORM_LINUX
void memory_patcher::free_process_handle(HANDLE handle) {
	if (handle != INVALID_HANDLE_VALUE) {
		CloseHandle(handle);
	}
}
#endif
