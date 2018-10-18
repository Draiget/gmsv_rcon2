#ifndef ZONTWELG_GMSVRCON2_UTILS_SSDK_H
#define ZONTWELG_GMSVRCON2_UTILS_SSDK_H

#include <datamap.h>
#include <const.h>
#include <tier1.h>
#include <tier2.h>
#include <tier3.h>
#include <tier1/checksum_crc.h>
#include <eiface.h>
#include <igameevents.h>
#include <iclient.h>
#include <netadr.h>
#include <inetmessage.h>
#include <inetchannel.h>
#include <igameevents.h>
#include <iachievementmgr.h>
#include <game/server/iplayerinfo.h>
#include <vphysics_interface.h>
#include <vphysics/vehicles.h>
#include <utllinkedlist.h>
#include <server_class.h>
#include <performance.h>
#include <vstdlib/cvar.h>
#include <utlbuffer.h>
#include <steam/steam_gameserver.h>

//------------------------------------------------------------------------
// Forward declarations
//------------------------------------------------------------------------
typedef unsigned int ra_listener_id;
const ra_listener_id INVALID_LISTENER_ID = 0xffffffff;

// enumerations for writing out the requests
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

struct ListenerStore_t
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
	virtual bool ShouldAcceptSocket( SocketHandle_t hSocket, const netadr_t &netAdr ) = 0; 
	virtual void OnSocketAccepted( SocketHandle_t hSocket, const netadr_t &netAdr, void** ppData ) = 0; 
	virtual void OnSocketClosed( SocketHandle_t hSocket, const netadr_t &netAdr, void* pData ) = 0;
};

class CRConServer : public ISocketCreatorListener
{
	
};

#endif
