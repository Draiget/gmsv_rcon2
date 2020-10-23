#include "utils_memory.h"
#include "platform.h"
#include "utils_mem_patch.h"

#include <cstring>
#include <cstdio>
#include <cstdlib>

#ifdef _PLATFORM_LINUX
#include <sys/mman.h>
#include <cerrno>
#endif

bool mem_protect_read_write_to(void* p_target_address, size_t size) {
#if _PLATFORM_LINUX
#ifdef MORE_DEBUG
	if (mprotect(reinterpret_cast<void*>(reinterpret_cast<unsigned>(p_target_address) & 0x0FFFFF000), size, PROT_READ | PROT_WRITE | PROT_EXEC) != 0) {
		Msg("[%s] mem_protect_read_write_to failed! Error: %s (%d)\n", GMSV_RCON2_PRINT_PREFIX, strerror(errno), errno);
		return false;
	}

	return true;
#else
	return mprotect(reinterpret_cast<void*>(reinterpret_cast<unsigned>(p_target_address) & 0x0FFFFF000), size, PROT_READ | PROT_WRITE | PROT_EXEC) == 0;
#endif
#else
	DWORD old_protect;
	return VirtualProtect(static_cast<unsigned char*>(p_target_address), size, PAGE_EXECUTE_READWRITE, &old_protect);
#endif
}

char* mem_alloc(unsigned u_code_size) {
#if _PLATFORM_LINUX
	return static_cast<char*>(mmap(nullptr, u_code_size, PROT_WRITE| PROT_READ | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0));
#else
	return static_cast<char*>(VirtualAlloc(nullptr, u_code_size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE));
#endif
}

void mem_insert_jmp(void** p_inject_address_loc, void** p_to_address) {
	const auto target = static_cast<unsigned char*>(*p_inject_address_loc);
	target[0] = 0xFF;
	target[1] = 0x25;
	*reinterpret_cast<void**>(&target[2]) = &(*p_to_address);
}

unsigned int util_string_to_signature(const char* str, char buffer[], size_t max_length) {
	unsigned int real_bytes = 0;
	const auto sig_length = strlen(str);

	for (unsigned int i = 0; i < sig_length; i++) {
		if (real_bytes >= max_length) {
			break;
		}

		buffer[real_bytes++] = static_cast<char>(static_cast<unsigned char>(str[i]));

		if (str[i] == '\\' && str[i + 1] == 'x') {
			if (i + 3 >= sig_length) {
				continue;
			}

			char s_byte[3];
			int r_byte;

			s_byte[0] = str[i + 2];
			s_byte[1] = str[i + 3];
			s_byte[2] = '\n';

#ifdef _PLATFORM_LINUX
			r_byte = static_cast<int>(strtoul(s_byte, nullptr, 16));
#else
			sscanf_s(s_byte, "%x", &r_byte);
#endif

			buffer[real_bytes - 1] = static_cast<char>(static_cast<unsigned char>(r_byte));

			i += 3;
		}
	}

	return real_bytes;
}

void* mem_find_signature_in_run_time(const char* p_signature, const char* p_dll) {
	if (!p_signature || p_signature[0] == 0) {
		return nullptr;
	}

	static char sig_buffer[512];
	const auto sig_len = util_string_to_signature(p_signature, sig_buffer, sizeof(sig_buffer));

	auto is_found = false;
	const auto module_info = zontwelg::memory_patcher::find_module(p_dll, &is_found);
	if (!is_found) {
		return nullptr;
	}

	auto ptr = reinterpret_cast<char*>(module_info->base_address);
	const auto end = ptr + module_info->base_size - 1;
	char* ret_address = nullptr;

	while (ptr < end) {
		auto found = true;

		for (unsigned int i = 0; i < sig_len; i++) {
			if (sig_buffer[i] != '\x2A' && sig_buffer[i] != ptr[i]) {
				found = false;
				break;
			}
		}

		if (found) {
			if (ret_address) {
				return reinterpret_cast<void*>(-1);
			}

			ret_address = ptr;
		}

		ptr++;
	}

	return ret_address;
}
