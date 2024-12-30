#include <iostream>
#include <string>
#include "steam_api.h"
#include "lua.hpp"

int lua_GetModPath(lua_State* L) {
	const char* strid = luaL_checkstring(L, 1);
	for (size_t i = 0; strid[i]; i++) {//不为数字返回空
		if (isdigit(strid[i]) == 0) return 0;
	}
	PublishedFileId_t id = std::stoll(strid);
	uint64 punSizeOnDisk;
	uint32 punTimeStamp;
	char path[512] = { 0 };
	bool result = SteamUGC()->GetItemInstallInfo(id, &punSizeOnDisk, path, sizeof(path), &punTimeStamp);
	if (!result) {//如果是假，返回空值
		return 0;
	}
	lua_pushstring(L, path);
	return 1;
}

int lua_GetSteamAPIInit(lua_State* L) {
	lua_pushboolean(L, SteamAPI_IsSteamRunning());
	return 1;
}

static luaL_Reg luaLibs[] = {
	{ "GetModPath", lua_GetModPath},
	{ "GetSteamAPIInit", lua_GetSteamAPIInit},
	{ NULL, NULL }
};

extern "C" __declspec(dllexport)
int luaopen_LuaSteamAPI(lua_State* L) {
	luaL_register(L, "LuaSteamAPI", luaLibs);  //注册函数，参数2是模块名
	return 1;
}
