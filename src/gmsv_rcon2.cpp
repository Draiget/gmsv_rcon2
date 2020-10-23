#include "gmsv_rcon2.h"
#include "platform.h"
#include "utils_ssdk.h"
#include "rcon_detours.h"

using namespace GarrysMod::Lua;

GMOD_MODULE_OPEN() {
	Msg("[%s] Loading module ...\n", GMSV_RCON2_PRINT_PREFIX);
	ConVar_Register();

	auto tier_lib = VStdLib_GetICVarFactory();
	ConnectTier1Libraries(&tier_lib, 1);
	ConnectTier2Libraries(&tier_lib, 1);
	ConnectTier3Libraries(&tier_lib, 1);

	Msg("[%s] Create hooks ...\n", GMSV_RCON2_PRINT_PREFIX);
	detour_rcon_init(state);

	Msg("[%s] Processing lua tables ...\n", GMSV_RCON2_PRINT_PREFIX);
	// ReSharper disable StringLiteralTypo
	LUA->PushSpecial(SPECIAL_GLOB);
		LUA->PushNumber(SERVERDATA_REQUESTVALUE);
		LUA->SetField(-2, "SERVERDATA_REQUESTVALUE");
		LUA->PushNumber(SERVERDATA_SETVALUE);
		LUA->SetField(-2, "SERVERDATA_SETVALUE");
		LUA->PushNumber(SERVERDATA_EXECCOMMAND);
		LUA->SetField(-2, "SERVERDATA_EXECCOMMAND");
		LUA->PushNumber(SERVERDATA_AUTH);
		LUA->SetField(-2, "SERVERDATA_AUTH");
		LUA->PushNumber(SERVERDATA_VPROF);
		LUA->SetField(-2, "SERVERDATA_VPROF");
		LUA->PushNumber(SERVERDATA_REMOVE_VPROF);
		LUA->SetField(-2, "SERVERDATA_REMOVE_VPROF");
		LUA->PushNumber(SERVERDATA_TAKE_SCREENSHOT);
		LUA->SetField(-2, "SERVERDATA_TAKE_SCREENSHOT");
		LUA->PushNumber(SERVERDATA_SEND_CONSOLE_LOG);
		LUA->SetField(-2, "SERVERDATA_SEND_CONSOLE_LOG");
	LUA->Pop();
	// ReSharper enable StringLiteralTypo

	Msg("[%s] Loading done\n", GMSV_RCON2_PRINT_PREFIX);
	return EXIT_SUCCESS;
}

GMOD_MODULE_CLOSE() {
	Msg("[%s] Closing module.\n", GMSV_RCON2_PRINT_PREFIX);
	detour_rcon_shutdown();
	return 0;
}
