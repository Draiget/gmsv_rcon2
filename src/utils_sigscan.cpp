#include "utils_sigscan.h"
#include <stdio.h>
#include <cstring>
#include "utils_memory.h"

unsigned int UTIL_StringToSignature2(const char *str, char buffer[], size_t maxlength)
{
	unsigned int real_bytes = 0;
	auto length = strlen(str);

	for (unsigned int i = 0; i < length; i++) {
		if (real_bytes >= maxlength) {
			break;
		}

		buffer[real_bytes++] = static_cast<unsigned char>(str[i]);

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

			buffer[real_bytes - 1] = static_cast<unsigned char>(r_byte);

			i += 3;
		}
	}

	return real_bytes;
}

void* UTIL_RuntimeSigScan(const char* pSignature, const char* pLibName) {
	if (!pSignature || pSignature[0] == 0){
		return nullptr;
	}

	static char pSig[512];
	auto sigLen = UTIL_StringToSignature2(pSignature, pSig, sizeof pSig);

	unsigned long moduleSize;
	auto address = MemGetModuleAddress(pLibName, &moduleSize);
	if (address == nullptr){
		return nullptr;
	}

	auto ptr = reinterpret_cast<char *>(address);
	auto end = ptr + moduleSize - 1;
	char *retn = nullptr;

	while (ptr < end) {
		auto found = true;

		for (unsigned int i = 0; i < sigLen; i++) {
			if (pSig[i] != '\x2A' && pSig[i] != ptr[i]) {
				found = false;
				break;
			}
		}

		if (found) {
			if (retn) {
				return MEM_INVALID_PTR;
			}

			retn = ptr;
		}

		ptr++;
	}

	return retn;
}
