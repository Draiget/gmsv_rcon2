#pragma once

#define MEM_INVALID_PTR (reinterpret_cast<void*>(-1))

#ifdef _PLATFORM_LINUX
#include <sys/types.h>
#endif

unsigned int util_string_to_signature(const char* str, char buffer[], size_t max_length);

bool mem_protect_read_write_to(void* p_target_address, size_t size);
void mem_insert_jmp(void** p_inject_address_loc, void** p_to_address);
char* mem_alloc(unsigned u_code_size);
void* mem_find_signature_in_run_time(const char* p_signature, const char* p_dll);
