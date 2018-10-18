#include "utils_net.h"

netadr_s* ListenerIdToAddress(CServerRemoteAccess* server, const ra_listener_id listener) {
	const auto listenerIDs = reinterpret_cast<CUtlLinkedList<ListenerStore_t, int>*>(
		reinterpret_cast<unsigned char*>(server) + 0x2C);

	if (listenerIDs->Count() <= listener){
		// Skipping invalid states
		return new netadr_s();
	}

	return &(*listenerIDs)[listener].adr;
}
