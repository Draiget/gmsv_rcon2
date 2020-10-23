#include "utils_net.h"
#include <utllinkedlist.h>

#ifdef _PLATFORM_LINUX
#define LISTENER_IDS_OFFSET 44
#else
#define LISTENER_IDS_OFFSET 0x2C
// 0x2c == 44 btw
#endif

netadr_s* listener_id_to_address(CServerRemoteAccess* server, const ra_listener_id listener) {
	const auto listener_ids = reinterpret_cast<CUtlLinkedList<ListenerStore_t, int>*>(
		reinterpret_cast<unsigned char*>(server) + LISTENER_IDS_OFFSET);

	if (listener_ids->Count() <= static_cast<int>(listener)) {
		// Skipping invalid states
		return new netadr_s();
	}

	return &(*listener_ids)[listener].adr;
}

ListenerStore_t* listener_from_address(CServerRemoteAccess* server, const ra_listener_id listener) {
	const auto listener_ids = reinterpret_cast<CUtlLinkedList<ListenerStore_t, int>*>(
		reinterpret_cast<unsigned char*>(server) + LISTENER_IDS_OFFSET);

	if (listener_ids->Count() <= static_cast<int>(listener)) {
		// Skipping invalid states
		return nullptr;
	}

	return &(*listener_ids)[listener];
}
