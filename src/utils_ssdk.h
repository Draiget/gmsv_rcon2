#pragma once

// Hide all bullshit regarding code we can't change
// ReSharper disable CppInconsistentNaming
// ReSharper disable CppPolymorphicClassWithNonVirtualPublicDestructor
// ReSharper disable IdentifierTypo
#include "platform.h"
#ifndef NO_MALLOC_OVERRIDE
#define NO_MALLOC_OVERRIDE
#endif

#ifdef _PLATFORM_LINUX
#pragma GCC diagnostic push 
#endif
#pragma warning(push, 0)

#include <datamap.h>
#include <const.h>
#include <tier1.h>
#include <tier2.h>
#include <tier3.h>
#include <iclient.h>
#include <netadr.h>
#include <vstdlib/cvar.h>
#include <utllinkedlist.h>
#include <utlbuffer.h>
#include <Lua/Interface.h>
#include <memalloc.h>

/*
* Forward declarations
*/

typedef unsigned int ra_listener_id;
const ra_listener_id INVALID_LISTENER_ID = 0xffffffff;

enum ServerDataRequestType_t
{
	SERVERDATA_REQUESTVALUE,
	SERVERDATA_SETVALUE,
	SERVERDATA_EXECCOMMAND,
	SERVERDATA_AUTH, // special RCON command to authenticate a connection
	SERVERDATA_VPROF, // subscribe to a vprof stream
	SERVERDATA_REMOVE_VPROF, // unsubscribe from a vprof stream
	SERVERDATA_TAKE_SCREENSHOT,
	SERVERDATA_SEND_CONSOLE_LOG,
};

struct ListenerStore_t  // NOLINT
{
	ra_listener_id	listenerID;
	bool			authenticated;
	bool			m_bHasAddress;
	netadr_t		adr;
};

class CServerRemoteAccess;
typedef int SocketHandle_t;

struct ISocketCreatorListener
{
public:
	// Methods to allow other classes to allocate data associated w/ sockets
	// Return false to disallow socket acceptance
	virtual bool ShouldAcceptSocket(SocketHandle_t hSocket, const netadr_t& netAdr) = 0;
	virtual void OnSocketAccepted(SocketHandle_t hSocket, const netadr_t& netAdr, void** ppData) = 0;
	virtual void OnSocketClosed(SocketHandle_t hSocket, const netadr_t& netAdr, void* pData) = 0;
};

class CRConServer : public ISocketCreatorListener
{

};

#pragma warning(pop)
#ifdef _PLATFORM_LINUX
#pragma GCC diagnostic pop
#endif

// ReSharper enable CppInconsistentNaming
// ReSharper restore CppPolymorphicClassWithNonVirtualPublicDestructor
// ReSharper restore IdentifierTypo
