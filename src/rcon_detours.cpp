#include "rcon_detours.h"
#include "platform.h"
#include "utils_ssdk.h"
#include "utils_macro.h"
#include "utils_net.h"
#include "detours_context.h"
#include "rcon_signatures.h"
#include "shared.h"
#include "utils_memory.h"
#include "validation.h"

#ifdef _PLATFORM_LINUX
#define ENGINE_MODULE_NAME "engine_srv.so"
#else
#define ENGINE_MODULE_NAME "engine.dll"
#endif

using namespace GarrysMod::Lua;
using namespace std;
using namespace zontwelg;

static lua_State* g_lua_state;

/*
 * Detours declarations
 */

DETOUR_DECLARE_A4(CServerRemoteAccess_WriteDataRequest, void, CRConServer*, p_server_ra, ra_listener_id, listener, const void*, buffer, int, buffer_size);
DETOUR_DECLARE_A1(CServerRemoteAccess_IsPassword, bool, const char*, p_password);
DETOUR_DECLARE_A2(CServerRemoteAccess_LogCommand, void, ra_listener_id, listener, const char*, msg);

/*
 * Helper functions
 */

bool get_string_helper(CUtlBuffer& cmd, char* outBuf, int bufSize) {
	outBuf[0] = 0;
	//cmd.GetString(outBuf, bufSize);
	cmd.GetStringManualCharCount(outBuf, bufSize);
	if (!cmd.IsValid()) {
		cmd.Purge();
		return false;
	}

	return true;
}

/*
 * Detours functions
 */

DETOUR_DECLARE_MEMBER_A4(CServerRemoteAccess_WriteDataRequest, void, CRConServer*, p_server_ra, ra_listener_id, listener, const void*, buffer, int, buffer_size) {
	// Check that the buffer contains at least the id and type
	if (buffer_size < static_cast<int>(2 * sizeof(int))) {
		return;
	}

	CUtlBuffer cmd(buffer, buffer_size, CUtlBuffer::READ_ONLY);
	shared::g_LastRconAddress = listener_id_to_address(reinterpret_cast<CServerRemoteAccess*>(this), listener);
	shared::g_LastListenerId = listener;

	// While there is commands to read
	while (static_cast<int>(cmd.TellGet()) < static_cast<int>(cmd.Size() - 2 * sizeof(int))) {
		// Parse out the buffer
		const auto request_id = cmd.GetInt();
		const auto request_type = cmd.GetInt();

		char str_buffer[256];
		if (!get_string_helper(cmd, str_buffer, sizeof(str_buffer))) {
			break;
		}

		auto state = g_lua_state;
		auto is_nil = true;
		auto hook_result = true;

		LUA->PushSpecial(SPECIAL_GLOB);
		LUA->GetField(-1, "hook");
		LUA->GetField(-1, "Call");
		LUA->PushString(GMSVRCON2_HOOK_WRITE_REQUEST);
		LUA->PushNil();
		LUA->PushNumber(listener);
		LUA->PushNumber(request_id);
		LUA->PushNumber(request_type);
		LUA->PushString(str_buffer);
		LUA->PushBool(shared::g_LastRconAddress->IsReservedAdr());
		LUA->PushString(shared::g_LastRconAddress->ToString(true));
		LUA->PushNumber(shared::g_LastRconAddress->GetPort());
		LUA->Call(9, 1);

		if (LUA->GetType(-1) == Type::BOOL) {
			is_nil = false;
			hook_result = LUA->GetBool(-1);
		}
		LUA->Pop(3);

		if (!is_nil && !hook_result) {
			return;
		}

		return DETOUR_MEMBER_CALL(CServerRemoteAccess_WriteDataRequest)(p_server_ra, listener, buffer, buffer_size);
	}

	DETOUR_MEMBER_CALL(CServerRemoteAccess_WriteDataRequest)(p_server_ra, listener, buffer, buffer_size);
}

DETOUR_DECLARE_MEMBER_A1(CServerRemoteAccess_IsPassword, bool, const char*, p_password) {
	if (!shared::g_LastRconAddress) {
		Msg("[" GMSV_RCON2_PRINT_PREFIX "] Skip hook '" GMSVRCON2_HOOK_CHECK_PASSWORD "': invalid pointer to g_LastRconAddress!\n");
		return DETOUR_MEMBER_CALL(CServerRemoteAccess_IsPassword)(p_password);
	}

	if (!shared::g_LastRconAddress->IsValid()) {
		Msg("[" GMSV_RCON2_PRINT_PREFIX "] Skip hook '" GMSVRCON2_HOOK_CHECK_PASSWORD "': network address is invalid\n");
		return DETOUR_MEMBER_CALL(CServerRemoteAccess_IsPassword)(p_password);
	}

	auto state = g_lua_state;
	auto is_nil = true;
	auto hook_result = true;

	LUA->PushSpecial(SPECIAL_GLOB);
	LUA->GetField(-1, "hook");
	LUA->GetField(-1, "Call");
	LUA->PushString(GMSVRCON2_HOOK_CHECK_PASSWORD);
	LUA->PushNil();
	LUA->PushString(p_password);
	LUA->PushNumber(shared::g_LastListenerId);
	LUA->PushBool(shared::g_LastRconAddress->IsReservedAdr());
	LUA->PushString(shared::g_LastRconAddress->ToString(true));
	LUA->PushNumber(shared::g_LastRconAddress->GetPort());
	LUA->Call(7, 1);

	if (LUA->GetType(-1) == Type::BOOL) {
		is_nil = false;
		hook_result = LUA->GetBool(-1);
	}
	LUA->Pop(3);

	if (!is_nil) {
		return hook_result;
	}

	return DETOUR_MEMBER_CALL(CServerRemoteAccess_IsPassword)(p_password);
}

DETOUR_DECLARE_MEMBER_A2(CServerRemoteAccess_LogCommand, void, ra_listener_id, listener, const char*, msg) {
	const auto listener_t = listener_from_address(reinterpret_cast<CServerRemoteAccess*>(this), listener);
	if (!listener_t) {
		Msg("[" GMSV_RCON2_PRINT_PREFIX "] Skip hook '" GMSVRCON2_HOOK_LOG_COMMAND "': unable to get listener data structure!\n");
		DETOUR_MEMBER_CALL(CServerRemoteAccess_LogCommand)(listener, msg);
		return;
	}

	const auto is_known_listener = listener_t->m_bHasAddress;
	if (is_known_listener && !listener_t->adr.IsValid()) {
		Msg("[" GMSV_RCON2_PRINT_PREFIX "] Skip hook '" GMSVRCON2_HOOK_LOG_COMMAND "': got invalid address (m_bHasAddress = 1)!\n");
		DETOUR_MEMBER_CALL(CServerRemoteAccess_LogCommand)(listener, msg);
		return;
	}

	auto is_nil = true;
	auto hook_result = true;
	auto state = g_lua_state;

	LUA->PushSpecial(SPECIAL_GLOB);
	LUA->GetField(-1, "hook");
	LUA->GetField(-1, "Call");
	LUA->PushString(GMSVRCON2_HOOK_LOG_COMMAND);
	LUA->PushNil();
	LUA->PushNumber(listener);
	LUA->PushString(msg);
	LUA->PushBool(is_known_listener);
	if (!is_known_listener) {
		LUA->PushBool(false);
		LUA->PushBool(false);
		LUA->PushBool(false);
	} else {
		LUA->PushBool(listener_t->adr.IsReservedAdr());
		LUA->PushString(listener_t->adr.ToString(true));
		LUA->PushNumber(listener_t->adr.GetPort());
	}
	LUA->Call(8, 1);

	if (LUA->GetType(-1) == Type::BOOL) {
		is_nil = false;
		hook_result = LUA->GetBool(-1);
	}
	LUA->Pop(3);

	if (!is_nil && !hook_result) {
		DETOUR_MEMBER_CALL(CServerRemoteAccess_LogCommand)(listener, msg);
	}
}

/*
 * Detours init/shutdown
 */

bool detour_rcon_init(lua_State* state) {
	g_lua_state = state;

	DETOUR_CREATE_MEMBER_EX(
		CServerRemoteAccess_IsPassword,
		"CServerRemoteAccess::IsPassword",
		SIG_C_SERVERREMOTEACCESS_IS_PASSWORD,
		ENGINE_MODULE_NAME,
		"[" GMSV_RCON2_PRINT_PREFIX "] Unable to detour IsPassword\n"
	);

	DETOUR_CREATE_MEMBER_EX(
		CServerRemoteAccess_WriteDataRequest,
		"CServerRemoteAccess::WriteDataRequest",
		SIG_C_SERVERREMOTEACCESS_WIRTE_DATA_REQUEST,
		ENGINE_MODULE_NAME,
		"[" GMSV_RCON2_PRINT_PREFIX "] Unable to detour WriteDataRequest\n"
	)

	DETOUR_CREATE_MEMBER_EX(
		CServerRemoteAccess_LogCommand,
		"CServerRemoteAccess::LogCommand",
		SIG_C_SERVERREMOTEACCESS_LOG_COMMAND,
		ENGINE_MODULE_NAME,
		"[" GMSV_RCON2_PRINT_PREFIX "] Unable to detour LogCommand\n"
	);

	Msg("[%s] RCON detoured!\n", GMSV_RCON2_PRINT_PREFIX);
	return true;
}

bool detour_rcon_shutdown() {
	DETOUR_DESTROY(CServerRemoteAccess_IsPassword);
	DETOUR_DESTROY(CServerRemoteAccess_WriteDataRequest);
	DETOUR_DESTROY(CServerRemoteAccess_LogCommand);
	return true;
}
