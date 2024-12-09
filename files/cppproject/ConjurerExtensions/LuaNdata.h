#pragma once
#include "ndata.h"
#include "ImageLoad.h"
#include "lua.hpp"

namespace lua {
	int DataWak(lua_State* L);
	int DestroyDataWak(lua_State* L);
	int AtDataWak(lua_State* L);
	int GetFileListDataWak(lua_State* L);
	int HasFileDataWak(lua_State* L);
	int GetImgToScale(lua_State* L);
	int GetImgFlatAndCropping(lua_State* L);
	luaL_Reg ndata_datawak[];
}
