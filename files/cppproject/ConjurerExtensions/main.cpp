#define NOMINMAX

#include <iostream>
#include <string>
#include <filesystem>

#define CBC 0
#define ECB 0
extern "C" {
#include <aes.h>
}

#include "LuaFilesApi.h"
#include "LuaRatioStr.h"
#include "lua.hpp"
#include "ImageLoad.h"
#include "LuaZip.h"
#include "LuaMemory.h"
#include "fn.h"
#include "LuaNdata.h"
#include "LuaImage.h"


static void DecryptBlock(uint8_t* blockDatas, size_t size, const uint8_t key[16], const uint8_t iv[16]) {
	//初始化AES_ctx对象
	AES_ctx ctx;
	AES_init_ctx_iv(&ctx, key, iv);
	AES_CTR_xcrypt_buffer(&ctx, blockDatas, size);
}

static std::vector<uint8_t> ReadBinFile(const char* path) {//读取二进制文件
	std::ifstream inputFile(path, std::ios::binary | std::ios::out);
	inputFile.seekg(0, std::ios::end);//移动文件指针以获得文件大小
	std::streamsize fileSize = inputFile.tellg();
	inputFile.seekg(0, std::ios::beg);

	std::vector<uint8_t> result(fileSize);
	inputFile.read((char*)result.data(), fileSize);
	return result;
}

namespace lua {
	int lua_AES128CTR(lua_State* L) {
		const char* path = luaL_checkstring(L, 1);
		if (!lua_istable(L, 2)) {//key
			lua_error(L);
			return 0;
		}
		if (!lua_istable(L, 3)) {//iv
			lua_error(L);
			return 0;
		}
		uint8_t key[16];//局部缓冲区
		uint8_t iv[16];
		for (size_t i = 1; i <= 16; i++) {//遍历表，以获得key和iv
			lua_rawgeti(L, 2, i);
			key[i - 1] = luaL_checkinteger(L, -1);
			lua_pop(L, 1);
		}
		for (size_t i = 1; i <= 16; i++) {
			lua_rawgeti(L, 3, i);
			iv[i - 1] = luaL_checkinteger(L, -1);
			lua_pop(L, 1);
		}
		std::vector<uint8_t> data = ReadBinFile(path);
		DecryptBlock(data.data(), data.size(), key, iv);
		lua_pushstring(L, std::string(data.begin(), data.end()).c_str());
		return 1;
	}

	int lua_AES128CTRToFile(lua_State* L) {
		const char* path = luaL_checkstring(L, 1);
		const char* toFile = luaL_checkstring(L, 2);
		if (!lua_istable(L, 3)) {//key
			lua_error(L);
			return 0;
		}
		if (!lua_istable(L, 4)) {//iv
			lua_error(L);
			return 0;
		}
		uint8_t key[16];//局部缓冲区
		uint8_t iv[16];
		for (size_t i = 1; i <= 16; i++) {//遍历表，以获得key和iv
			lua_rawgeti(L, 3, i);
			key[i - 1] = luaL_checkinteger(L, -1);
			lua_pop(L, 1);
		}
		for (size_t i = 1; i <= 16; i++) {
			lua_rawgeti(L, 4, i);
			iv[i - 1] = luaL_checkinteger(L, -1);
			lua_pop(L, 1);
		}
		std::vector<uint8_t> data = ReadBinFile(path);
		DecryptBlock(data.data(), data.size(), key, iv);
		auto WriteBinFile = [](const std::string& path, const std::vector<uint8_t>& BinData) {//写入二进制文件
			std::ofstream outputFile(path, std::ios::binary | std::ios::trunc);
			outputFile.write((const char*)BinData.data(), BinData.size());
			outputFile.close();
			};
		WriteBinFile(toFile, data);
		return 0;
	}

	int lua_UTF8StringSub(lua_State* L) {
		pinyin::Utf8String s1 = pinyin::Utf8String(luaL_checkstring(L, 1));
		int pos1 = luaL_checkinteger(L, 2) - 1;
		int pos2 = luaL_checkinteger(L, 3) - 1;
		if (pos1 - 1 > pos2) {
			lua_pushstring(L, luaL_checkstring(L, 1));
			return 1;
		}
		std::string result = "";
		for (int i = pos1; i <= pos2 && i < s1.size(); i++) {//安全检查
			result += s1[i];
		}
		lua_pushstring(L, result.c_str());
		return 1;
	}

	int lua_UTF8StringSize(lua_State* L) {
		pinyin::Utf8String s1 = pinyin::Utf8String(luaL_checkstring(L, 1));
		lua_pushnumber(L, s1.size());
		return 1;
	}

	int lua_UTF8StringChars(lua_State* L) {
		lua_newtable(L);//新建一个表
		pinyin::Utf8String s1 = pinyin::Utf8String(luaL_checkstring(L, 1));
		for (size_t i = 0; i < s1.size(); i++) {
			lua_pushstring(L, s1[i].c_str());
			lua_rawseti(L, -2, i + 1);
		}
		return 1;
	}

	int lua_FinnishToEnLower(lua_State* L) {
		pinyin::Utf8String s1 = pinyin::Utf8String(luaL_checkstring(L, 1));
		for (size_t i = 0; i < s1.size(); i++) {
			if (s1[i] == "Å" || s1[i] == "å") {
				s1[i] = "a";
			}
			else if (s1[i] == "Ä" || s1[i] == "ä") {
				s1[i] = "a";
			}
			else if (s1[i] == "Ö" || s1[i] == "ö") {
				s1[i] = "o";
			}
		}
		lua_pushstring(L, s1.ToStream().c_str());
		return 1;
	}

	int lua_SetClipboard(lua_State* L) {
		std::string str = luaL_checkstring(L, 1);
		lua_pushboolean(L, fn::SetClipboard(str));
		return 1;
	}

	int lua_GetClipboard(lua_State* L) {
		lua_pushstring(L, fn::GetClipboard().c_str());
		return 1;
	}

	int lua_SetDllDirectory(lua_State* L) {
		const char* str = luaL_checkstring(L, 1);
		lua_pushboolean(L, SetDllDirectoryA(str));
		return 1;
	}

	int lua_ConcatStr(lua_State* L) {
		int size = lua_gettop(L);
		std::string result;
		for (int i = 1; i <= size; i++) {
			result += luaL_checkstring(L, i);
		}
		lua_pushstring(L, result.c_str());
		return 1;
	}

	int lua_FlipImageLoadAndWrite(lua_State* L) {
		const char* FileStr = luaL_checkstring(L, 1);
		const char* WritePath = luaL_checkstring(L, 2);
		stbi_set_flip_vertically_on_load(true);
		image::stb_image img(FileStr);
		if (img.GetImageData() == nullptr) {
			luaL_error(L, "no found file");
			lua_pushboolean(L, false);
			stbi_set_flip_vertically_on_load(false);
			return 1;
		}
		auto data = img.GetImageData();
		auto height = img.GetHeight();
		auto width = img.GetWidth();
		auto channels = img.GetChannels();

		for (int y = 0; y < height; ++y) {
			for (int x = 0; x < width / 2; ++x) {
				for (int c = 0; c < channels; ++c) {
					int left = (y * width + x) * channels + c;
					int right = (y * width + (width - 1 - x)) * channels + c;
					std::swap(data[left], data[right]);
				}
			}
		}

		img.WritePng(WritePath);
		lua_pushboolean(L, true);
		stbi_set_flip_vertically_on_load(false);
		return 1;
	}

	int lua_System(lua_State* L) {
		const char* command = luaL_checkstring(L, 1);
		lua_pushnumber(L, system(command));
		return 1;
	}

	int lua_ANSIToUTF8(lua_State* L) {
		const char* str = luaL_checkstring(L, 1);
		lua_pushstring(L, fn::GbkToUtf8(str).c_str());
		return 1;
	}

	int lua_RegLMGetValue(lua_State* L) {
		const char* key = luaL_checkstring(L, 1);
		const char* valuekey = luaL_checkstring(L, 2);
		std::string result = fn::GetRegistryValue(HKEY_LOCAL_MACHINE, key, valuekey);
		if (result == "") {
			return 0;
		}
		lua_pushstring(L, result.c_str());
		return 1;
	}

	int lua_PngScaleToFile(lua_State* L) {
		const char* PngPath = luaL_checkstring(L, 1);
		const char* ToFile = luaL_checkstring(L, 2);
		int Width = luaL_checkinteger(L, 3);
		int Height = luaL_checkinteger(L, 4);
		image::stb_image img(PngPath);
		if (img.GetIsNullptr()) {
			lua_pushboolean(L, false);
			return 1;
		}
		unsigned char* dstData = new unsigned char[Width * Height * img.GetChannels()];//缓冲区
		image::resize_nearest_neighbor(img.GetImageData(), img.GetWidth(), img.GetHeight(), dstData, Width, Height, img.GetChannels());

		lua_pushboolean(L, static_cast<bool>(stbi_write_png(ToFile, Width, Height, img.GetChannels(), dstData, Width * img.GetChannels())));
		delete[] dstData;
		return 1;
	}

	int lua_PngFlatAndCroppingToFile(lua_State* L) {
		const char* PngPath = luaL_checkstring(L, 1);
		const char* ToFile = luaL_checkstring(L, 2);
		int Width = luaL_checkinteger(L, 3);
		int Height = luaL_checkinteger(L, 4);
		image::stb_image img(PngPath);
		if (img.GetIsNullptr()) {
			lua_pushboolean(L, false);
			return 1;
		}
		lua_pushboolean(L, FlatAndCroppingImg(img, Width, Height).WritePng(ToFile));
		return 1;
	}

	int lua_RGBAPng(lua_State* L) {
		const char* ToFile = luaL_checkstring(L, 1);
		int Width = luaL_checkinteger(L, 2);
		int Height = luaL_checkinteger(L, 3);
		int r = luaL_checkinteger(L, 4);
		int g = luaL_checkinteger(L, 5);
		int b = luaL_checkinteger(L, 6);
		int a = luaL_checkinteger(L, 7);
		unsigned char* image = new unsigned char[Width * Height * 4];

		//填充图像数据，逐像素设置颜色
		for (int y = 0; y < Height; y++) {
			for (int x = 0; x < Width; x++) {
				int index = (y * Width + x) * 4;
				image[index] = r;   //R
				image[index + 1] = g; //G
				image[index + 2] = b; //B
				image[index + 3] = a; //A
			}
		}
		lua_pushboolean(L, static_cast<bool>(stbi_write_png(ToFile, Width, Height, 4, image, Width * 4)));
		delete[] image;
		return 1;
	}

	int lua_SameElemListStr(lua_State* L) {
		int size = luaL_checkinteger(L, 1);
		int len = luaL_checkinteger(L, 2);
		const char* str = luaL_checkstring(L, 3);
		char* result = new char[len * size + size];//字符串缓冲区申请
		char* resultPtr = result;
		for (int i = 1; i < size; i++) {
			for (int j = 0; j < len; j++) {
				*resultPtr = str[j];
				resultPtr++;
			}
			*resultPtr = ',';
			resultPtr++;
		}
		for (int j = 0; j < len; j++) {
			*resultPtr = str[j];
			resultPtr++;
		}
		*resultPtr = '\0';
		lua_pushstring(L, result);
		delete[] result;
		return 1;
	}
}

//提供给lua的函数
static luaL_Reg luaLibs[] = {
	{ "CurrentPath", lua::lua_CurrentPath},
	{ "GetDirectoryPath", lua::lua_GetDirectoryPath},
	{ "GetDirectoryPathAll", lua::lua_GetDirectoryPathAll},
	{ "GetAbsPath", lua::lua_GetAbsPath},
	{ "PathGetFileName", lua::lua_PathGetFileName},
	{ "PathGetFileType", lua::lua_PathGetFileType},
	{ "PathExists", lua::lua_PathExists},
	{ "CreateDir", lua::lua_CreateDir},
	{ "CreateDirs", lua::lua_CreateDirs},
	{ "Rename", lua::lua_Rename},
	{ "Remove", lua::lua_Remove},
	{ "RemoveAll", lua::lua_RemoveAll},

	{ "NewBoolPtr", lua::lua_NewBoolPtr},
	{ "SetBoolPtrV", lua::lua_SetBoolPtrV},
	{ "GetBoolPtrV", lua::lua_GetBoolPtrV},

	{ "NewIntPtr", lua::lua_NewIntPtr},
	{ "SetIntPtrV", lua::lua_SetIntPtrV},
	{ "GetIntPtrV", lua::lua_GetIntPtrV},
	{ "Free", lua::lua_Free},

	{ "UTF8StringSize", lua::lua_UTF8StringSize},
	{ "UTF8StringSub", lua::lua_UTF8StringSub},
	{ "UTF8StringChars", lua::lua_UTF8StringChars},
	{ "ConcatStr", lua::lua_ConcatStr},
	{ "FinnishToEnLower", lua::lua_FinnishToEnLower},

	{ "Ratio", lua::lua_Ratio},
	{ "PartialRatio", lua::lua_PartialRatio},
	{ "PinyinRatio", lua::lua_PinyinRatio},
	{ "AbsPartialPinyinRatio", lua::lua_AbsPartialPinyinRatio},

	{ "SetClipboard", lua::lua_SetClipboard},
	{ "GetClipboard", lua::lua_GetClipboard},
	{ "SetDllDirectory", lua::lua_SetDllDirectory},

	{ "FlipImageLoadAndWrite", lua::lua_FlipImageLoadAndWrite},
	{ "Uncompress", lua::lua_Uncompress},
	{ "System", lua::lua_System},
	{ "RegLMGetValue", lua::lua_RegLMGetValue},

	{ "ANSIToUTF8" , lua::lua_ANSIToUTF8},

	{ "DataWak" , lua::DataWak},

	{ "RGBAPng" , lua::lua_RGBAPng},
	{ "PngScaleToFile" , lua::lua_PngScaleToFile},
	{ "PngFlatAndCroppingToFile", lua::lua_PngFlatAndCroppingToFile},

	{ "SameElemListStr", lua::lua_SameElemListStr},

	{ "NullImageCreate", luaImage::NullImageCreate},
	{ "ImageCreate", luaImage::ImageCreate},

	{ "AES128CTR", lua::lua_AES128CTR},
	{ "AES128CTRToFile", lua::lua_AES128CTRToFile},
	{ NULL, NULL }
};

extern "C" __declspec(dllexport)
int luaopen_ConjurerExtensions(lua_State* L) {
	luaL_newmetatable(L, "lua_stb_image");
	lua_pushvalue(L, -1);

	//将元表的__index字段指向自身，然后注册函数，以实现类似调用类方法的形式
	lua_setfield(L, -2, "__index");//会执行弹出操作
	luaL_register(L, NULL, luaImage::ImageFn);

	//创建元表
	luaL_newmetatable(L, "lua_ndata_dataWak");
	lua_pushvalue(L, -1);
	//将元表的__index字段指向自身，然后注册函数，以实现类似调用类方法的形式
	lua_setfield(L, -2, "__index");//会执行弹出操作
	luaL_register(L, NULL, lua::ndata_datawak);

	luaL_register(L, "ConjurerExtensions", luaLibs);  //注册函数，参数2是模块名
	return 1;
}
