#include "os_definions.h"
#include "detours_context.h"
#include "utils_asm.h"

#include <cstdio>
#include "utils_memory.h"
#include "utils_sigscan.h"

CDetourContext *CDetourContext::CreateDetour(void *pDetourCallbackFn, void **pTrampoline, const char *pDetourName, const char *pSignature, const char *pLibName) {
	auto detour = new CDetourContext(pDetourCallbackFn, pTrampoline, pDetourName, pSignature, pLibName);
	if (!detour) {
		return nullptr;
	}

	if (detour->Init()){ 
		return detour;
	}

	delete detour;
	return nullptr;
}

CDetourContext::CDetourContext(void *pDetourCallbackFn, void **pTrampoline, const char *pDetourName, const char *pSignature, const char *pLibName) {
	m_Enabled = false;
	m_Detoured = false;
	m_Detour_Address = nullptr;
	m_Detour_Trampoline = nullptr;

	this->m_Detour_Name = pDetourName;
	this->m_Detour_Signature = pSignature;
	this->m_Detour_LibName = pLibName;
	this->m_Detour_Callback = pDetourCallbackFn;
	this->m_Trampoline = pTrampoline;
}

bool CDetourContext::Init() {
	if (!CreateDetour()) {
		m_Enabled = false;
		return m_Enabled;
	}

	m_Enabled = true;
	return m_Enabled;
}


void CDetourContext::Destroy() {
	DeleteDetour();
	delete this;
}

bool CDetourContext::IsEnabled() const {
	return m_Enabled;
}

bool CDetourContext::CreateDetour() {
	m_Detour_Address = UTIL_RuntimeSigScan(m_Detour_Signature, m_Detour_LibName);
	if (!m_Detour_Address) {
		return false;
	}

	if (m_Detour_Address == MEM_INVALID_PTR) {
		return false;
	}

	// Get first detour bytes lenght (if dest buffer is nullptr, we are not copy anything, just iterate count)
	m_Detour_Restore.bytes = AsmCopyBytes(static_cast<unsigned char*>(m_Detour_Address), nullptr, OP_JMP_SIZE + 1);

	// Copy first original function bytes to patch array
	for (size_t i = 0; i < m_Detour_Restore.bytes; i++) {
		m_Detour_Restore.patch[i] = static_cast<unsigned char*>(m_Detour_Address)[i];
	}

	Rewind();
	return true;
}

void CDetourContext::Rewind(char* pOutbase, char* pOutptr) {
	if (pOutbase != nullptr) {
		AsmCopyBytes(static_cast<unsigned char*>(m_Detour_Address), reinterpret_cast<unsigned char*>(pOutptr), m_Detour_Restore.bytes);
	}

	pOutptr += m_Detour_Restore.bytes;

	if (pOutbase) {
		// Copy JMP to first bytes of original memory
		*pOutptr = static_cast<unsigned char>( OP_JMP );
	}

	++pOutptr;
	unsigned int call = pOutptr - pOutbase;

	if (pOutbase) {
		*reinterpret_cast<int *>(pOutptr) = 0;
	}

	pOutptr += sizeof(int);
	auto oldptr = pOutptr;
	pOutptr = pOutbase + call;

	if (pOutbase) {
		*reinterpret_cast<int*>(pOutptr) = 
			reinterpret_cast<long>(static_cast<unsigned char*>(m_Detour_Address) + m_Detour_Restore.bytes) - (reinterpret_cast<long>(pOutbase) + call + 4);
	}

	pOutptr = oldptr;

	if (pOutbase == nullptr) {
		m_Detour_Trampoline = pOutptr = pOutbase = static_cast<char*>(MemAlloc(pOutptr - pOutbase));
		Rewind(pOutbase, pOutptr);
	}

	*m_Trampoline = m_Detour_Trampoline;
}

void CDetourContext::DeleteDetour() {
	if (m_Detoured) {
		DisableDetour();
	}

	if (!m_Detour_Trampoline) {
		return;
	}

	m_Detour_Trampoline = nullptr;
}

void CDetourContext::EnableDetour() {
	if (m_Detoured) {
		return;
	}

	MemProtectReadWriteTo(m_Detour_Address, 20);
	MemInsertJmp(&m_Detour_Address, &m_Detour_Callback);

	m_Detoured = true;
}

void CDetourContext::DisableDetour() {
	if (!m_Detoured) {
		return;
	}

	MemProtectReadWriteTo(m_Detour_Address, 20);

	auto addr = static_cast<unsigned char*>(m_Detour_Address);
	for (size_t i=0; i < m_Detour_Restore.bytes; i++) {
		addr[i] = m_Detour_Restore.patch[i];
	}

	m_Detoured = false;
}