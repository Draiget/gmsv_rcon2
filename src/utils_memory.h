#ifndef ZONTWELG_GMSVRCON2_UTILS_MEMORY_H
#define ZONTWELG_GMSVRCON2_UTILS_MEMORY_H

#define MEM_INVALID_PTR reinterpret_cast<void *>(-1)

void MemProtectReadWriteTo(void* pTargetAddress, size_t dSize);
void MemInsertJmp(void** pInjectAddressLoc, void** pToAddress);
char* MemAlloc(unsigned uCodeSize);
char* MemGetModuleAddress(const char* pLibName, unsigned long* uSize);
void *MemFindSignatureInRunTime(const char *pSignature, const char *pDLL);

#endif
