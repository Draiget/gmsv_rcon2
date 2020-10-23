#pragma once
#include "utils_ssdk.h"

netadr_s* listener_id_to_address(CServerRemoteAccess* server, ra_listener_id listener);
ListenerStore_t* listener_from_address(CServerRemoteAccess* server, const ra_listener_id listener);
