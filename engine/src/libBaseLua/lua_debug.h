#pragma once
#include "libBaseCommon/base_common.h"
#include "lua_facade.h"

bool startLuaDebug(lua_State* pL, const char* szIP, uint16_t nPort);
void updateLuaDebug(lua_State* pL);