#include "rcon_detours.h"
#include "utils_ssdk.h"
#include "utils_macro.h"
#include "rcon_signatures.h"
#include "utils_net.h"
#include "shared.h"
#include <Lua/Interface.h>
#include "detours_context.h"
#include "utils_sigscan.h"

using namespace GarrysMod::Lua;
using namespace std;

static lua_State* s_LuaState;
static unsigned long* s_mListenerIDs = nullptr;

DETOUR_DECLARE_A4( CServerRemoteAccess_CheckPassword, void, CRConServer*, pServerRA, ra_listener_id, listener, int, requestId, const char*, password);
DETOUR_DECLARE_A4( CServerRemoteAccess_WriteDataRequest, void, CRConServer*, pServerRA, ra_listener_id, listener, const void*, buffer, int, bufferSize);
DETOUR_DECLARE_A1( CServerRemoteAccess_IsPassword, bool, const char*, pPassword);
// DETOUR_DECLARE_A2( CServerRemoteAccess_LookupValue, bool, const char*, variable, CUtlBuffer&, value);
DETOUR_DECLARE_A2( CServerRemoteAccess_LogCommand, void, ra_listener_id, listener, const char*, msg);

DETOUR_DECLARE_MEMBER_A4(CServerRemoteAccess_WriteDataRequest, void, CRConServer*, pServerRA, ra_listener_id, listener, const void*, buffer, int, bufferSize) {
	// check that the buffer contains at least the id and type
	if (bufferSize < 2 * sizeof(int)) {
		return;
	}

	CUtlBuffer cmd(buffer, bufferSize, CUtlBuffer::READ_ONLY);
	auto invalidRequest = false;

	// while there is commands to read
	while (invalidRequest == false && static_cast<int>(cmd.TellGet()) < static_cast<int>(cmd.Size() - 2 * sizeof(int))) {
		// parse out the buffer
		auto requestID = cmd.GetInt();
		auto requestType = cmd.GetInt();

		auto state = s_LuaState;
		auto isNil = true;
		auto hookResult = true;
		auto ip = ListenerIDToAddress(reinterpret_cast<CServerRemoteAccess *>(this), listener);
		char strBuffer[256];
		cmd.GetStringManualCharCount( strBuffer, 256 );
		
		LUA->PushSpecial(SPECIAL_GLOB);
			LUA->GetField(-1, "hook");
			LUA->GetField(-1, "Call");
			LUA->PushString(GMSVRCON2_HOOK_WRITE_REQUEST);
			LUA->PushNil();
			LUA->PushNumber(listener);
			LUA->PushNumber(requestID);
			LUA->PushNumber(requestType);
			LUA->PushString(strBuffer);
			LUA->PushBool(ip->IsReservedAdr());
			LUA->PushString(ip->ToString(true));
			LUA->PushNumber(ip->GetPort());
			LUA->Call(9, 1);

			if (LUA->GetType(-1) == Type::BOOL) {
				isNil = false;
				hookResult = LUA->GetBool(-1);
			}
		LUA->Pop(3);
		
		if (!isNil && !hookResult){
			return;
		}

		return DETOUR_MEMBER_CALL(CServerRemoteAccess_WriteDataRequest)(pServerRA, listener, buffer, bufferSize);
	}

	DETOUR_MEMBER_CALL(CServerRemoteAccess_WriteDataRequest)(pServerRA, listener, buffer, bufferSize);
}

DETOUR_DECLARE_MEMBER_A4( CServerRemoteAccess_CheckPassword, void, CRConServer*, pServerRA, ra_listener_id, listener, int, requestId, const char*, password) {
	_G::g_LastRconAddress = ListenerIDToAddress(reinterpret_cast<CServerRemoteAccess *>(this), listener);
	_G::g_LastListenerId = listener;
	DETOUR_MEMBER_CALL(CServerRemoteAccess_CheckPassword)(pServerRA, listener, requestId, password);
}

DETOUR_DECLARE_MEMBER_A1(CServerRemoteAccess_IsPassword, bool, const char*, pPassword) {
	if (!_G::g_LastRconAddress) {
		printf("[gmsv_rcon] Skip hook '" GMSVRCON2_HOOK_CHECK_PASSWORD "': invalid pointer to g_LastRconAddress!\n");
		return DETOUR_MEMBER_CALL(CServerRemoteAccess_IsPassword)(pPassword);
	}

	if (!_G::g_LastRconAddress->IsValid()){
		printf("[gmsv_rcon] Skip hook '" GMSVRCON2_HOOK_CHECK_PASSWORD "': network address is invalid\n");
		return DETOUR_MEMBER_CALL(CServerRemoteAccess_IsPassword)(pPassword);
	}

	auto state = s_LuaState;
	auto isNil = true;
	auto hookResult = true;

	LUA->PushSpecial(SPECIAL_GLOB);
		LUA->GetField(-1, "hook");
		LUA->GetField(-1, "Call");
		LUA->PushString(GMSVRCON2_HOOK_CHECK_PASSWORD);
		LUA->PushNil();
		LUA->PushString(pPassword);
		LUA->PushNumber(_G::g_LastListenerId);
		LUA->PushBool(_G::g_LastRconAddress->IsReservedAdr());
		LUA->PushString(_G::g_LastRconAddress->ToString(true));
		LUA->PushNumber(_G::g_LastRconAddress->GetPort());
		LUA->Call(7, 1);

		if (LUA->GetType(-1) == Type::BOOL) {
			isNil = false;
			hookResult = LUA->GetBool(-1);
		}
	LUA->Pop(3);

	if (!isNil) {
		return hookResult;
	}

	return DETOUR_MEMBER_CALL(CServerRemoteAccess_IsPassword)(pPassword);
}

/*DETOUR_DECLARE_MEMBER_A2( CServerRemoteAccess_LookupValue, bool, const char*, variable, CUtlBuffer&, value) {
	Assert(value.IsText());

	return DETOUR_MEMBER_CALL(CServerRemoteAccess_LookupValue)(variable, value);
}*/

DETOUR_DECLARE_MEMBER_A2( CServerRemoteAccess_LogCommand, void, ra_listener_id, listener, const char*, msg) {
	auto server = reinterpret_cast<CServerRemoteAccess *>(this);
	auto m_ListenerIDs = reinterpret_cast<CUtlLinkedList<ListenerStore_t, int>*>(reinterpret_cast<unsigned char*>(server) + 0x2C);
	if (!m_ListenerIDs){
		printf("[gmsv_rcon] Skip hook '" GMSVRCON2_HOOK_LOG_COMMAND "': invalid pointer m_ListenerIDs!\n");
		DETOUR_MEMBER_CALL(CServerRemoteAccess_LogCommand)(listener, msg);
		return;
	}

	auto isKnownListener = listener >= 0 && listener < static_cast<ra_listener_id>(m_ListenerIDs->Count()) && (*m_ListenerIDs)[listener].m_bHasAddress; 
	auto address = (*m_ListenerIDs)[listener].adr;

	if (!address.IsValid()){
		printf("[gmsv_rcon] Skip hook '" GMSVRCON2_HOOK_LOG_COMMAND "': got invalid address!\n");
		DETOUR_MEMBER_CALL(CServerRemoteAccess_LogCommand)(listener, msg);
		return;
	}

	auto isNil = true;
	auto hookResult = true;
	auto state = s_LuaState;

	LUA->PushSpecial( SPECIAL_GLOB );
		LUA->GetField(-1, "hook");
		LUA->GetField(-1, "Call");
		LUA->PushString(GMSVRCON2_HOOK_LOG_COMMAND); 
		LUA->PushNil();
		LUA->PushNumber(listener);
		LUA->PushString(msg);
		LUA->PushBool(isKnownListener);
		if (!isKnownListener){
			LUA->PushBool(false);
			LUA->PushBool(false);
			LUA->PushBool(false);
		} else {
			LUA->PushBool(address.IsReservedAdr());
			LUA->PushString(address.ToString(true));
			LUA->PushNumber(address.GetPort());
		}
		LUA->Call(8, 1);

		if (LUA->GetType(-1) == Type::BOOL) {
			isNil = false;
			hookResult = LUA->GetBool(-1);
		}
	LUA->Pop(3);
		
	if (!isNil && !hookResult){
		DETOUR_MEMBER_CALL(CServerRemoteAccess_LogCommand)(listener, msg);
	}
}

bool DetourRconInit(lua_State* state) {
	s_LuaState = state;

	DETOUR_CREATE_MEMBER(
		CServerRemoteAccess_WriteDataRequest,
			"CServerRemoteAccess::WriteDataRequest",
			SIG_CSERVERREMOTEACCESS_WRITEDATAREQUEST,
			"engine.dll"
	)

	DETOUR_CREATE_MEMBER(
		CServerRemoteAccess_CheckPassword,
			"CServerRemoteAccess::CheckPassword",
			SIG_CSERVERREMOTEACCESS_CHECKPASSWORD,
			"engine.dll"
	);

	DETOUR_CREATE_MEMBER(
		CServerRemoteAccess_IsPassword,
			"CServerRemoteAccess::IsPassword",
			SIG_CSERVERREMOTEACCESS_ISPASSWORD,
			"engine.dll"
	);

	/*DETOUR_CREATE_MEMBER(
		CServerRemoteAccess_LookupValue,
			"CServerRemoteAccess::LookupValue",
			SIG_CSERVERREMOTEACCESS_LOOKUPVALUE,
			"engine.dll"
	);*/

	DETOUR_CREATE_MEMBER(
		CServerRemoteAccess_LogCommand,
			"CServerRemoteAccess::LogCommand",
			SIG_CSERVERREMOTEACCESS_LOGCOMMAND,
			"engine.dll"
	);

	printf("[gmsv_rcon2] Rcon detoured!\n");
	return true;
}

bool DetourRconShutdown() {
	DETOUR_DESTROY(CServerRemoteAccess_WriteDataRequest);
	DETOUR_DESTROY(CServerRemoteAccess_CheckPassword);
	DETOUR_DESTROY(CServerRemoteAccess_IsPassword);
	// DETOUR_DESTROY(CServerRemoteAccess_LookupValue);
	DETOUR_DESTROY(CServerRemoteAccess_LogCommand);
	return true;
}
