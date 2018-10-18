#include "utils_memory.h"
#include "os_definions.h"
#include <winnt.h>
#include <comdef.h>
#include "utils_mempatch.h"

#pragma comment(lib, "Kernel32.lib")

void MemProtectReadWriteTo(void* pTargetAddress, size_t dSize) {
	DWORD lpflOldProtect;
	VirtualProtect(static_cast<unsigned char*>(pTargetAddress), dSize, PAGE_EXECUTE_READWRITE, &lpflOldProtect);
}

void MemInsertJmp(void** pInjectAddressLoc, void** pToAddress) {
	const auto target = static_cast<unsigned char *>(*pInjectAddressLoc);
	target[0] = 0xFF;
	target[1] = 0x25;
	*reinterpret_cast<void **>(&target[2]) = &(*pToAddress);
}

char* MemAlloc(unsigned uCodeSize) {
	return static_cast<char*>(VirtualAlloc(nullptr, uCodeSize, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE));
}

char* MemGetModuleAddress(const char* pLibName, unsigned long* uSize) {
	const auto snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, GetCurrentProcessId());
	if (snapshot == INVALID_HANDLE_VALUE){
		return nullptr;
	}

	char neededName[256];

	if (pLibName) {
		strcpy_s(neededName, pLibName);
		_strupr_s(neededName);
	}

	MODULEENTRY32W moduleInfo = { 0 };
	moduleInfo.dwSize = sizeof(MODULEENTRY32W);

	if (!Module32FirstW(snapshot, &moduleInfo)){
		CloseHandle(snapshot);
		return nullptr;
	}

	do {
		const _bstr_t moduleNameStr(moduleInfo.szModule);

		if (!_stricmp(moduleNameStr, neededName)) {
			CloseHandle(snapshot);

			*uSize = moduleInfo.modBaseSize;
			return reinterpret_cast<char *>(moduleInfo.modBaseAddr);
		}
	} while (Module32NextW(snapshot, &moduleInfo));

	CloseHandle(snapshot);

	*uSize = 0;
	return nullptr;
}

unsigned int MemStringToSignature(const char *str, char buffer[], size_t maxlength)
{
	unsigned int realBytes = 0;
	const auto length = strlen(str);

	for (unsigned int i = 0; i < length; i++) {
		if (realBytes >= maxlength) {
			break;
		}

		buffer[realBytes++] = static_cast<unsigned char>(str[i]);

		if (str[i] == '\\' && str[i + 1] == 'x') {
			if (i + 3 >= length) {
				continue;
			}

			char s_byte[3];
			int r_byte;

			s_byte[0] = str[i + 2];
			s_byte[1] = str[i + 3];
			s_byte[2] = '\n';

			sscanf_s(s_byte, "%x", &r_byte);

			buffer[realBytes - 1] = static_cast<unsigned char>(r_byte);

			i += 3;
		}
	}

	return realBytes;
}

void* MemFindSignatureInRunTime(const char* pSignature, const char* pDLL) {
	if (!pSignature || pSignature[0] == 0) {
		return nullptr;
	}

	static char pSig[512];
	const auto siglen = MemStringToSignature(pSignature, pSig, sizeof(pSig));

	static CMemoryPatcher *pMemPatch = nullptr;
	if (!pMemPatch) {
		pMemPatch = new CMemoryPatcher;

		if (!pMemPatch->Init()) {
			return nullptr;
		}
	}

	static tagMODULEENTRY32 modlentry;

	if (!pMemPatch->FindModuleInProcess(pDLL, GetCurrentProcessId(), &modlentry)) {
		return nullptr;
	}

	auto ptr = reinterpret_cast<char *>(modlentry.modBaseAddr);
	const auto end = ptr + modlentry.modBaseSize - 1;
	char *retn = nullptr;

	while (ptr < end) {
		auto found = true;

		for (unsigned int i = 0; i < siglen; i++) {
			if (pSig[i] != '\x2A' && pSig[i] != ptr[i]) {
				found = false;
				break;
			}
		}

		if (found) {
			if (retn) {
				return reinterpret_cast<void *>(-1);
			}

			retn = ptr;
		}

		ptr++;
	}

	return retn;
}

