#ifndef ZONTWELG_GMSVRCON2_UTILS_MEMPATCH_H
#define ZONTWELG_GMSVRCON2_UTILS_MEMPATCH_H

#include "os_definions.h"
#include <tlhelp32.h>
#include <winnt.h>

class CMemoryPatcher
{
public:
	CMemoryPatcher();
	~CMemoryPatcher();

	bool Init();
	bool FindModuleInProcess(const char* module_name, DWORD pid, tagMODULEENTRY32* modentry) const;

private:
	static void FreeProcessHandle(HANDLE hSnapShot);
	bool ModulesLookup(DWORD pid, const char* name, DWORD addr, tagMODULEENTRY32* modentry) const;

	HMODULE hKernel32;
	HANDLE(WINAPI *pfnCreateToolhelp32Snapshot) (DWORD, DWORD);
	BOOL(WINAPI *pfnProcess32First) (HANDLE, tagPROCESSENTRY32*);
	BOOL(WINAPI *pfnProcess32Next) (HANDLE, tagPROCESSENTRY32*);
	BOOL(WINAPI *pfnModule32First) (HANDLE, tagMODULEENTRY32*);
	BOOL(WINAPI *pfnModule32Next) (HANDLE, tagMODULEENTRY32*);
};

#endif
