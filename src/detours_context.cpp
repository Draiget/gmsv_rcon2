#include "detours_context.h"
#include "utils_memory.h"
#include "utils_asm.h"
#include <cstdio>
#include "platform.h"
#include <cstring>
#include <cstdlib>
#include "validation.h"
#include "utils_ssdk.h"

#ifdef _PLATFORM_LINUX
#include <sys/mman.h>
#include <cerrno>
#endif

using namespace zontwelg;

c_detour_context* c_detour_context::create(
	void* p_detour_callback_fn, 
	void** p_trampoline, 
	const char* p_detour_name, 
	const char* p_signature, 
	const char* p_lib_name) 
{
	auto detour = new c_detour_context(
		p_detour_callback_fn,
		p_trampoline,
		p_detour_name, 
		p_signature, 
		p_lib_name);

	if (!detour) {
		return nullptr;
	}

	if (detour->init()) {
		return detour;
	}

	delete detour;
	return nullptr;
}

bool c_detour_context::is_enabled() const {
	return m_enabled_;
}

void c_detour_context::enable_detour() {
	if (m_detoured_) {
		return;
	}

	if (!mem_protect_read_write_to(m_detour_address, 20)) {
		Msg("[%s] Detour '%s' failed to change memory permissions!\n", GMSV_RCON2_PRINT_PREFIX, m_detour_name_);
		return;
	}

	mem_insert_jmp(&m_detour_address, &m_detour_callback);
#ifdef MORE_DEBUG
	Msg("[%s] Detour '%s' jmp inserted [at=%p, to=%p]\n", GMSV_RCON2_PRINT_PREFIX, m_detour_name_, m_detour_address, m_detour_callback);
#endif

	m_detoured_ = true;
}

void c_detour_context::disable_detour() {
	if (!m_detoured_) {
		return;
	}

	mem_protect_read_write_to(m_detour_address, 20);

	const auto address = static_cast<unsigned char*>(m_detour_address);
	for (size_t i = 0; i < m_detour_restore_.bytes; i++) {
		address[i] = m_detour_restore_.patch[i];
	}

	m_detoured_ = false;
}

void c_detour_context::destroy() {
	delete_detour();
	delete this;
}

c_detour_context::c_detour_context(
	void* p_detour_callback_fn, 
	void** p_trampoline, 
	const char* p_detour_name,
	const char* p_signature, 
	const char* p_lib_name)
{
	m_enabled_ = false;
	m_detoured_ = false;
	m_detour_address = nullptr;
	m_detour_trampoline = nullptr;

	this->m_detour_name_ = p_detour_name;
	this->m_detour_signature_ = p_signature;
	this->m_detour_lib_name_ = p_lib_name;
	this->m_detour_callback = p_detour_callback_fn;
	this->m_trampoline_ = p_trampoline;
}

bool c_detour_context::init() {
	if (!create_detour()) {
		m_enabled_ = false;
		return m_enabled_;
	}

	m_enabled_ = true;
	return m_enabled_;
}

bool c_detour_context::create_detour() {
	m_detour_address = mem_find_signature_in_run_time(m_detour_signature_, m_detour_lib_name_);
	if (!m_detour_address) {
#ifdef MORE_DEBUG
		Msg("[%s] Detour signature not found\n", GMSV_RCON2_PRINT_PREFIX);
#endif
		return false;
	}

	if (m_detour_address == MEM_INVALID_PTR) {
		return false;
	}

	copy_function(&m_detour_restore_, m_detour_address);
	rewind();

#ifdef MORE_DEBUG
	Msg("[%s] Detour '%s' enabled\n", GMSV_RCON2_PRINT_PREFIX, m_detour_name_);
#endif
	return true;
}

void c_detour_context::copy_function(detour_patch_t* reference, void* src_address) {
	if (!is_valid_ptr(reference)) {
		return;
	}

	// Get first detour bytes length (if dest buffer is nullptr, we are not copy anything, just iterate count)
	reference->bytes = asm_copy_bytes(static_cast<unsigned char*>(src_address), nullptr, OP_JMP_SIZE + 1);

	// Copy first original function bytes to patch array
	for (size_t i = 0; i < reference->bytes; i++) {
		reference->patch[i] = static_cast<unsigned char*>(src_address)[i];
	}
}

void c_detour_context::delete_detour() {
	if (m_detoured_) {
		disable_detour();
	}

	if (!m_detour_trampoline) {
		return;
	}

	m_detour_trampoline = nullptr;
}

void c_detour_context::rewind(char* p_out_base, char* p_out_ptr) {
	if (p_out_base != nullptr) {
		asm_copy_bytes(static_cast<unsigned char*>(m_detour_address), reinterpret_cast<unsigned char*>(p_out_ptr), m_detour_restore_.bytes);
	}

	p_out_ptr += m_detour_restore_.bytes;

	if (p_out_base) {
		// Copy JMP to first bytes of original memory
		*p_out_ptr = static_cast<char>(static_cast<unsigned char>(OP_JMP));
	}

	++p_out_ptr;
	const unsigned int call = p_out_ptr - p_out_base;

	if (p_out_base) {
		*reinterpret_cast<int*>(p_out_ptr) = 0;
	}


	p_out_ptr += sizeof(int);
	const auto old_ptr = p_out_ptr;
	p_out_ptr = p_out_base + call;

	if (p_out_base) {
		*reinterpret_cast<int*>(p_out_ptr) =
			static_cast<int>(
				reinterpret_cast<long>(static_cast<unsigned char*>(m_detour_address) + m_detour_restore_.bytes) - (reinterpret_cast<long>(p_out_base) + call + 4));
	}

	p_out_ptr = old_ptr;

	if (p_out_base == nullptr) {
		m_detour_trampoline = p_out_ptr = p_out_base = static_cast<char*>(mem_alloc(p_out_ptr - p_out_base));
#ifdef _PLATFORM_LINUX
		if (m_detour_trampoline == MAP_FAILED) {
			Msg("[%s] Detour memory allocate failed (MAP_FAILED)! Error: %s (%d)\n", GMSV_RCON2_PRINT_PREFIX, strerror(errno), errno);
			exit(EXIT_FAILURE);
		}
#endif
		rewind(p_out_base, p_out_ptr);
	}

	*m_trampoline_ = m_detour_trampoline;
}
