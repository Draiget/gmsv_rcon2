#undef GetCommandLine
#undef ReadConsoleInput
#define _CRT_SECURE_NO_WARNINGS

#undef RegCreateKey
#undef RegCreateKeyEx
#undef RegOpenKey
#undef RegOpenKeyEx
#undef RegQueryValue
#undef RegQueryValueEx
#undef RegSetValue
#undef RegSetValueEx
#undef INVALID_HANDLE_VALUE

#include <cstdio>
#include <cerrno>
#include "tier0/memdbgon.h"
#include "utils_mempatch.h"

CMemoryPatcher::CMemoryPatcher()
{
	hKernel32 = nullptr;
	pfnCreateToolhelp32Snapshot = nullptr;
	pfnProcess32First = nullptr;
	pfnProcess32Next = nullptr;
	pfnModule32First = nullptr;
	pfnModule32Next = nullptr;
}

CMemoryPatcher::~CMemoryPatcher()
{
	if (hKernel32) {
		FreeLibrary(hKernel32);
	}
}

bool CMemoryPatcher::Init()
{
	if (hKernel32) {
		return true;
	}

	if ((hKernel32 = LoadLibraryA("kernel32.dll")) == nullptr) {
		printf("[MemPatch] Could not open kernel32.dll: %s\n", strerror(errno));
		return false;
	}

	if ((pfnCreateToolhelp32Snapshot = reinterpret_cast<HANDLE(WINAPI *)(DWORD, DWORD)>(GetProcAddress(hKernel32, "CreateToolhelp32Snapshot"))) == nullptr) {
		printf("[MemPatch] Could not get CreateToolhelp32Snapshot address\n");
		return false;
	}

	if ((pfnProcess32First = reinterpret_cast<BOOL(WINAPI *)(HANDLE, tagPROCESSENTRY32*)>(GetProcAddress(hKernel32, "Process32First"))) == nullptr) {
		printf("[MemPatch] Could not get Process32First address\n");
		return false;
	}

	if ((pfnProcess32Next = reinterpret_cast<BOOL(WINAPI *)(HANDLE, tagPROCESSENTRY32*)>(GetProcAddress(hKernel32, "Process32Next"))) == nullptr) {
		printf("[MemPatch] Could not get Process32Next address\n");
		return false;
	}

	if ((pfnModule32First = reinterpret_cast<BOOL(WINAPI *)(HANDLE, tagMODULEENTRY32*)>(GetProcAddress(hKernel32, "Module32First"))) == nullptr) {
		printf("[MemPatch] Could not get Module32First address\n");
		return false;
	}

	if ((pfnModule32Next = reinterpret_cast<BOOL(WINAPI *)(HANDLE, tagMODULEENTRY32*)>(GetProcAddress(hKernel32, "Module32Next"))) == nullptr) {
		printf("[MemPatch] Could not get pfnModule32Next address\n");
		return false;
	}

	return true;
}

void CMemoryPatcher::FreeProcessHandle(HANDLE hSnapShot)
{
	if (hSnapShot != INVALID_HANDLE_VALUE) {
		CloseHandle(hSnapShot);
	}
}

bool CMemoryPatcher::FindModuleInProcess(const char *module_name, DWORD pid, tagMODULEENTRY32 *modentry) const
{
	if (pid) {
		return ModulesLookup(pid, module_name, 0, modentry);
	}

	auto hAllProcesses = INVALID_HANDLE_VALUE;
	tagPROCESSENTRY32 hProcEntry = { 0 };
	hProcEntry.dwSize = sizeof(tagPROCESSENTRY32);

	if ((hAllProcesses = (*pfnCreateToolhelp32Snapshot)(TH32CS_SNAPPROCESS, 0)) == INVALID_HANDLE_VALUE) {
		printf("[MemPatch] CreateToolhelp32Snapshot: %s\n", strerror(errno));
		return false;
	}

	if ((*pfnProcess32First)(hAllProcesses, &hProcEntry)) {
		while (true) {
			if (hProcEntry.th32ProcessID != 0) {
				if (ModulesLookup(hProcEntry.th32ProcessID, module_name, 0, modentry)) {
					FreeProcessHandle(hAllProcesses);
					return true;
				}
			}

			hProcEntry.dwSize = sizeof(tagPROCESSENTRY32);
			if (!(*pfnProcess32Next)(hAllProcesses, &hProcEntry)) {
				break;
			}
		}
	}

	FreeProcessHandle(hAllProcesses);
	return false;
}

bool CMemoryPatcher::ModulesLookup(DWORD pid, const char *name, DWORD addr, tagMODULEENTRY32 *modentry) const
{
	char neededName[256], module_name[256];
	auto hAllModules = INVALID_HANDLE_VALUE;

	if (name) {
		strcpy_s(neededName, name);
		_strupr_s(neededName);
	}

	if ((hAllModules = (*pfnCreateToolhelp32Snapshot)(TH32CS_SNAPMODULE, pid)) == INVALID_HANDLE_VALUE) {
		return false;
	}

	modentry->dwSize = sizeof(tagMODULEENTRY32);

	if ((*pfnModule32First)(hAllModules, modentry)) {
		while (true) {
			modentry->dwSize = sizeof(tagMODULEENTRY32);

			if (name) {
				strcpy_s(module_name, modentry->szModule);
				_strupr_s(module_name);

				if (!_stricmp(module_name, neededName)) {
					FreeProcessHandle(hAllModules);
					return true;
				}
			}
			else {
				if (addr >= reinterpret_cast<DWORD>(modentry->modBaseAddr) && addr <= reinterpret_cast<DWORD>(modentry->modBaseAddr) + modentry->modBaseSize) {
					FreeProcessHandle(hAllModules);
					return true;
				}
			}

			if (!(*pfnModule32Next)(hAllModules, modentry)) {
				break;
			}
		}
	}

	FreeProcessHandle(hAllModules);
	return false;
}
