// For VS2017
#pragma comment(lib, "legacy_stdio_definitions.lib")

#include <Lua/Interface.h>
#include <cstdlib>
#include "rcon_detours.h"
#include <cstdio>
#include "utils_ssdk.h"
#include "utils_memory.h"
#include "rcon_signatures.h"
using namespace GarrysMod::Lua;

GMOD_MODULE_OPEN() {
	printf("[gmsv_rcon2] Initializing ...");
	Log("[gmsv_rcon2] Initializing ...\n");
	ConVar_Register();

	auto tierLib = VStdLib_GetICVarFactory();
	ConnectTier1Libraries(&tierLib, 1);
	ConnectTier2Libraries(&tierLib, 1);
	ConnectTier3Libraries(&tierLib, 1);

	// Create RCON functions detour
	DetourRconInit(state);

	LUA->PushSpecial( SPECIAL_GLOB );
		LUA->PushNumber( SERVERDATA_REQUESTVALUE );
		LUA->SetField( -2, "SERVERDATA_REQUESTVALUE" );
		LUA->PushNumber( SERVERDATA_SETVALUE );
		LUA->SetField( -2, "SERVERDATA_SETVALUE" );
		LUA->PushNumber( SERVERDATA_EXECCOMMAND );
		LUA->SetField( -2, "SERVERDATA_EXECCOMMAND" );
		LUA->PushNumber( SERVERDATA_AUTH );
		LUA->SetField( -2, "SERVERDATA_AUTH" );
		LUA->PushNumber( SERVERDATA_VPROF );
		LUA->SetField( -2, "SERVERDATA_VPROF" );
		LUA->PushNumber( SERVERDATA_REMOVE_VPROF );
		LUA->SetField( -2, "SERVERDATA_REMOVE_VPROF" );
		LUA->PushNumber( SERVERDATA_TAKE_SCREENSHOT );
		LUA->SetField( -2, "SERVERDATA_TAKE_SCREENSHOT" );
		LUA->PushNumber( SERVERDATA_SEND_CONSOLE_LOG );
		LUA->SetField( -2, "SERVERDATA_SEND_CONSOLE_LOG" );
	LUA->Pop();

	Log("[gmsv_rcon2] Loading done");
	return EXIT_SUCCESS;
}

GMOD_MODULE_CLOSE() {
	// Cleanup detours on server shutdown (map reload cause this method too)
	DetourRconShutdown();
	return EXIT_SUCCESS;
}