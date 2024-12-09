#include "ImageLoad.h"
#include "lua.hpp"

namespace luaImage {
	int NullImageCreate(lua_State* L);
	int ImageCreate(lua_State* L);
	int DestroyImage(lua_State* L);
	int ImageGetPixel(lua_State* L);
	int ImageGetPixelHex(lua_State* L);
	int ImageGetWidth(lua_State* L);
	int ImageGetHeight(lua_State* L);
	int ImageGetChannels(lua_State* L);
	int ImageSetPixel(lua_State* L);
	luaL_Reg ImageFn[];
}
