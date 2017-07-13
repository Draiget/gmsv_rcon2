#ifndef ZONTWELG_GMSVRCON2_DETOURS_H
#define ZONTWELG_GMSVRCON2_DETOURS_H

#include <Lua/Interface.h>

#define GMSVRCON2_HOOK_CHECK_PASSWORD	"OnRconCheckPassword"
#define GMSVRCON2_HOOK_WRITE_REQUEST	"OnRconWriteRequest"

bool DetourRconInit(lua_State* state);
bool DetourRconShutdown();

#endif
