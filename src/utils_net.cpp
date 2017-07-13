#include "utils_net.h"

netadr_s* ListenerIDToAddress(CServerRemoteAccess* server, ra_listener_id listener) {
	ListenerStore_t* store;
	__asm
	{
		push ebx
		push eax
		push edx
		push edi
		mov edi, listener
		mov ebx, server
		mov eax, [ebx + 0x2C]			// m_ListenerIDs
		lea edx, [edi + edi * 0x08]
		lea ebx, [eax + edx * 0x04]		// m_ListenerIDs[ listener ]
		mov store, ebx
		pop edi
		pop edx
		pop eax
		pop ebx
	}

	return &store->adr;
}
