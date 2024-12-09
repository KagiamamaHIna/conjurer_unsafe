#include "LuaImage.h"

namespace luaImage {//此命名空间的函数用于暴露给lua
	int NullImageCreate(lua_State* L) {
		int width = luaL_checkinteger(L, 1);
		int height = luaL_checkinteger(L, 2);
		int channel = luaL_checkinteger(L, 3);
		image::stb_image* stb_image_ptr = (image::stb_image*)lua_newuserdata(L, sizeof(image::stb_image));
		new (stb_image_ptr) image::stb_image(width, height, channel);

		luaL_getmetatable(L, "lua_stb_image");//设置元表属性
		lua_setmetatable(L, -2);
		return 1;
	}

	//stb_image
	int ImageCreate(lua_State* L) {//提供给lua的构造函数
		const char* str = luaL_checkstring(L, 1);
		image::stb_image* stb_image_ptr = (image::stb_image*)lua_newuserdata(L, sizeof(image::stb_image));
		new (stb_image_ptr) image::stb_image(str);
		if (stb_image_ptr->GetImageData() == nullptr) {
			luaL_error(L, "no found file");
			return 0;
		}

		luaL_getmetatable(L, "lua_stb_image");//设置元表属性
		lua_setmetatable(L, -2);
		return 1;
	}

	int DestroyImage(lua_State* L) {//析构函数
		image::stb_image* img = (image::stb_image*)lua_touserdata(L, 1);
		img->~stb_image();//调用析构函数，防止内存泄漏
		return 0;
	}

	int ImageGetPixel(lua_State* L) {
		image::stb_image* img = (image::stb_image*)luaL_checkudata(L, 1, "lua_stb_image");
		int x = luaL_checkint(L, 2);
		int y = luaL_checkint(L, 3);
		if (x >= img->GetWidth() or y >= img->GetHeight()) {
			luaL_error(L, "bounds index out of image");
			return 0;
		}
		image::rgba result = img->GetPixel(x, y);
		for (int i = 0; i < img->GetChannels(); i++) {
			lua_pushinteger(L, result.rgbaArray[i]);
		}
		return img->GetChannels();
	}

	int ImageSetPixel(lua_State* L) {
		image::stb_image* img = (image::stb_image*)luaL_checkudata(L, 1, "lua_stb_image");
		int topsize = lua_gettop(L);
		if (topsize < img->GetChannels() + 2) {
			std::string result = "image channel is " + std::to_string(img->GetChannels());
			result += ", you must input: " + std::to_string(img->GetChannels() + 3) + " args";
			result += "\n you input arg count is:" + std::to_string(topsize);
			luaL_error(L, result.c_str());
			return 0;
		}
		int x = luaL_checkint(L, 2);
		int y = luaL_checkint(L, 3);
		image::rgba color;
		color.channels = img->GetChannels();
		for (int i = 4; i <= topsize; i++) {
			color.rgbaArray[i - 4] = luaL_checkint(L, i);
		}
		img->SetPixel(x, y, color);
		return 0;
	}

	int ImageGetPixelHex(lua_State* L) {
		image::stb_image* img = (image::stb_image*)luaL_checkudata(L, 1, "lua_stb_image");
		int x = luaL_checkint(L, 2);
		int y = luaL_checkint(L, 3);
		if (x >= img->GetWidth() or y >= img->GetHeight()) {
			luaL_error(L, "bounds index out of image");
			return 0;
		}
		image::rgba result = img->GetPixel(x, y);
		lua_pushinteger(L, result.GetHex());

		return 1;
	}

	int ImageGetWidth(lua_State* L) {//等价实现
		image::stb_image* img = (image::stb_image*)luaL_checkudata(L, 1, "lua_stb_image");
		lua_pushinteger(L, img->GetWidth());
		return 1;
	}

	int ImageGetHeight(lua_State* L) {
		image::stb_image* img = (image::stb_image*)luaL_checkudata(L, 1, "lua_stb_image");
		lua_pushinteger(L, img->GetHeight());
		return 1;
	}

	int ImageGetChannels(lua_State* L) {
		image::stb_image* img = (image::stb_image*)luaL_checkudata(L, 1, "lua_stb_image");
		lua_pushinteger(L, img->GetChannels());
		return 1;
	}

	int ImageWritePng(lua_State* L) {
		image::stb_image* img = (image::stb_image*)luaL_checkudata(L, 1, "lua_stb_image");
		const char* path = luaL_checkstring(L, 2);
		img->WritePng(path);
		return 0;
	}

	luaL_Reg ImageFn[] = {
		{"GetChannels",ImageGetChannels},
		{"GetHeight",ImageGetHeight},
		{"GetWidth",ImageGetWidth},
		{"GetPixel",ImageGetPixel},
		{"GetPixelHex",ImageGetPixelHex},
		{"SetPixel", ImageSetPixel},
		{"WritePng", ImageWritePng},
		{ "__gc",DestroyImage}
	};
}
