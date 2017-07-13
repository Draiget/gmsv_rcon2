#include "utils_memory.h"
#include "os_definions.h"
#include <tlhelp32.h>
#include <winnt.h>
#include <comdef.h>

#pragma comment(lib, "Kernel32.lib")

void MemProtectReadWriteTo(void* pTargetAddress, size_t dSize) {
	DWORD lpflOldProtect;
	VirtualProtect(static_cast<unsigned char*>(pTargetAddress), dSize, PAGE_EXECUTE_READWRITE, &lpflOldProtect);
}

void MemInsertJmp(void** pInjectAddressLoc, void** pToAddress) {
	auto target = static_cast<unsigned char *>(*pInjectAddressLoc);
	target[0] = 0xFF;
	target[1] = 0x25;
	*reinterpret_cast<void **>(&target[2]) = &(*pToAddress);
}

char* MemAlloc(unsigned uCodeSize) {
	return static_cast<char*>(VirtualAlloc(nullptr, uCodeSize, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE));
}

char* MemGetModuleAddress(const char* pLibName, unsigned long* uSize) {
	auto snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, GetCurrentProcessId());
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
		_bstr_t moduleNameStr(moduleInfo.szModule);

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

