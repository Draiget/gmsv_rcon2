#pragma once
#include <Lua/Interface.h>

// ReSharper disable IdentifierTypo
#define GMSVRCON2_HOOK_CHECK_PASSWORD	"OnRconCheckPassword"
#define GMSVRCON2_HOOK_WRITE_REQUEST	"OnRconWriteRequest"
#define GMSVRCON2_HOOK_LOG_COMMAND		"OnRconLogCommand"
// ReSharper restore IdentifierTypo

bool detour_rcon_init(lua_State* state);
bool detour_rcon_shutdown();
