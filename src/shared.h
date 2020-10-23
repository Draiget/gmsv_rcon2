#pragma once

/*
 * Forward declarations of SSDK stuff
 */

typedef struct netadr_s netadr_t;
typedef unsigned int ra_listener_id;

namespace zontwelg 
{
	class shared
	{
	public:
		static netadr_s* g_LastRconAddress;
		static ra_listener_id g_LastListenerId;
	};
}
