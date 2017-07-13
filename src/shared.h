#ifndef ZONTWELG_GMSVRCON2_SHARED_H
#define ZONTWELG_GMSVRCON2_SHARED_H

// Forward declarations of SSDK stuff
typedef struct netadr_s netadr_t;
typedef unsigned int ra_listener_id;

class _G
{
public:
	static netadr_s* g_LastRconAddress;
	static ra_listener_id g_LastListenerId;
};

#endif
