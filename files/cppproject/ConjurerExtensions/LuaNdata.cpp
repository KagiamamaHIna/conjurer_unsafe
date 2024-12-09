#include "LuaNdata.h"

namespace lua {
	luaL_Reg ndata_datawak[] = {
		{ "At", AtDataWak },
		{ "GetFileList", GetFileListDataWak },
		{ "HasFile" , HasFileDataWak },
		{ "GetImgToScale", GetImgToScale},
		{ "GetImgFlatAndCropping", GetImgFlatAndCropping},
		{ "__gc", DestroyDataWak }
	};

	int DataWak(lua_State* L) {//提供给lua的构造函数
		const char* str = luaL_checkstring(L, 1);
		ndata::DataWak* DataWak = (ndata::DataWak*)lua_newuserdata(L, sizeof(ndata::DataWak));
		try {
			new (DataWak) ndata::DataWak(str);
		}
		catch (ndata::DataExceptionBase& e) {
			lua_pop(L, 1);//构造数据失败，弹出表数据，代表lua应该进行析构
			lua_pushnil(L);//返回空数据
			std::string errorText = typeid(e).name();
			errorText += ":";
			errorText += e.what();
			lua_pushstring(L, errorText.c_str());//返回错误信息
			lua_pushnumber(L, e.ErrorPos);
			return 3;
		}
		luaL_getmetatable(L, "lua_ndata_dataWak");//设置元表属性
		lua_setmetatable(L, -2);
		return 1;
	}

	int DestroyDataWak(lua_State* L) {
		ndata::DataWak* data = (ndata::DataWak*)lua_touserdata(L, 1);
		data->~DataWak();//调用析构函数，防止内存泄漏
		return 0;
	}

	int AtDataWak(lua_State* L) {
		ndata::DataWak* data = (ndata::DataWak*)lua_touserdata(L, 1);
		std::string key = luaL_checkstring(L, 2);
		if (data->HasFile(key)) {
			lua_pushstring(L, ndata::VecU8ToStr(data->operator[](key)).c_str());
			return 1;
		}
		return 0;
	}

	int HasFileDataWak(lua_State* L) {
		ndata::DataWak* data = (ndata::DataWak*)lua_touserdata(L, 1);
		std::string key = luaL_checkstring(L, 2);
		lua_pushboolean(L, data->HasFile(key));
		return 1;
	}

	int GetFileListDataWak(lua_State* L) {
		ndata::DataWak* data = (ndata::DataWak*)lua_touserdata(L, 1);
		lua_newtable(L);
		int count = 1;
		for (auto& v : data->umap()) {
			lua_pushstring(L, v.first.c_str());
			lua_rawseti(L, -2, count);
			count++;
		}
		return 1;
	}

	int GetImgToScale(lua_State* L) {
		ndata::DataWak* data = (ndata::DataWak*)lua_touserdata(L, 1);
		std::string key = luaL_checkstring(L, 2);//获取文件键
		const char* ToFile = luaL_checkstring(L, 3);//获取写入路径
		int Width = luaL_checkinteger(L, 4);//获取宽
		int Height = luaL_checkinteger(L, 5);//获取高

		if (!data->HasFile(key)) {//不存在文件就退出
			lua_pushboolean(L, false);
			return 1;
		}

		image::stb_image img = image::stb_image((data->operator[](key)));
		if (img.GetIsNullptr()) {
			lua_pushboolean(L, false);
			return 1;
		}
		//目前点采样效果最好
		unsigned char* dstData = new unsigned char[Width * Height * img.GetChannels()];//缓冲区
		image::resize_nearest_neighbor(img.GetImageData(), img.GetWidth(), img.GetHeight(), dstData, Width, Height, img.GetChannels());

		lua_pushboolean(L, static_cast<bool>(stbi_write_png(ToFile, Width, Height, img.GetChannels(), dstData, Width * img.GetChannels())));
		delete[] dstData;
		return 1;
	}

	int GetImgFlatAndCropping(lua_State* L) {
		ndata::DataWak* data = (ndata::DataWak*)lua_touserdata(L, 1);
		std::string key = luaL_checkstring(L, 2);//获取文件键
		const char* ToFile = luaL_checkstring(L, 3);//获取写入路径
		int Width = luaL_checkinteger(L, 4);//获取宽
		int Height = luaL_checkinteger(L, 5);//获取高

		if (!data->HasFile(key)) {//不存在文件就退出
			lua_pushboolean(L, false);
			return 1;
		}

		image::stb_image img = image::stb_image((data->operator[](key)));
		if (img.GetIsNullptr()) {
			lua_pushboolean(L, false);
			return 1;
		}
		lua_pushboolean(L, FlatAndCroppingImg(img, Width, Height).WritePng(ToFile));
		return 1;
	}
}
