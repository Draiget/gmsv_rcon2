#include "utils_net.h"

netadr_s* ListenerIDToAddress(CServerRemoteAccess* server, ra_listener_id listener) {
	auto m_ListenerIDs = reinterpret_cast<CUtlLinkedList<ListenerStore_t, int>*>(reinterpret_cast<unsigned char*>(server) + 0x2C);
	return &(*m_ListenerIDs)[listener].adr;
}
