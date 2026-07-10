local ffi = require("ffi")
ffi.cdef([[
int SetDllDirectoryA(const char* lpPathName);

void NVFSMHInit();

uint32_t NVFSFindVFSystem();
uint32_t NVFSFindModFileAddVFS();
uint32_t NVFSFindModDiskFileDevice();
uint32_t NVFSUnknownVFS();

union ssoUnion {
	char* buffer;
	char sso_buffer[16];
};

struct std_string { /* VC++ std::string */
    union ssoUnion data;
    size_t size;
    size_t capacity;
};
typedef void __fastcall ModFileAddVFS(void* unknownVFS, void* ModDiskFileDeviceObj, int is_workshop, struct std_string* true_path_non_slash, struct std_string* true_path_non_slash, struct std_string* vfpath, void* ModDiskFileDeviceCaching);
typedef void* __thiscall unknownVFSFn(void*);
typedef void* __thiscall ModDiskFileDevice(char* this, struct std_string* a2, struct std_string* a3);

]])
ffi.C.SetDllDirectoryA("mods/conjurer_unsafe/files/module/")
local NVFS = ffi.load("NVFSPatcher")

local function CheckNullptr(ptr, name)
	if ptr == 0 then
		error(name .. " is nullptr")
	end
end

local VFSystem = NVFS.NVFSFindVFSystem()
CheckNullptr(VFSystem, "VFSystem")
local ptrModFileAddVFS = NVFS.NVFSFindModFileAddVFS()
CheckNullptr(ptrModFileAddVFS, "ModFileAddVFS")
local ptrModDiskFileDevice = NVFS.NVFSFindModDiskFileDevice()
CheckNullptr(ptrModDiskFileDevice, "ModDiskFileDevice")
local UnknownVFS = NVFS.NVFSUnknownVFS()
CheckNullptr(UnknownVFS, "UnknownVFS")

---函数转换
local cModFileAddVFS = ffi.cast("ModFileAddVFS*", ptrModFileAddVFS)
local cModDiskFileDevice = ffi.cast("ModDiskFileDevice*", ptrModDiskFileDevice)

---@return ffi.cdata*
local function getUnknownVFSField()
	local Vftable = ffi.cast("char**", UnknownVFS)[0]
    local fn = ffi.cast("unknownVFSFn*", ffi.cast("char**", (Vftable + 36))[0])
	return fn(ffi.cast("void*", UnknownVFS))
end

local UnknownVFSField = getUnknownVFSField()

---实现std string用于API传递
---@class std_string

---@param str string
---@return std_string
local function ToStdString(str)
    local stdstrPtr = ffi.new("struct std_string[1]")
	local stdstr = stdstrPtr[0]
    stdstr.size = str:len()
    if str:len() >= 16 then
        stdstr.data.buffer = ffi.new("char[?]", str:len() + 1)
        for i = 0, str:len() - 1 do
            stdstr.data.buffer[i] = str:byte(i + 1, i + 1)
        end
        stdstr.data.buffer[str:len()] = 0
        stdstr.capacity = str:len()
    else
        stdstr.capacity = 15
		if str:len() == 0 then
            stdstr.size = 0
            stdstr.data.sso_buffer[0] = 0
			return stdstrPtr
		end
        for i = 0, str:len() - 1 do
            stdstr.data.sso_buffer[i] = str:byte(i + 1, i + 1)
        end
        stdstr.data.sso_buffer[str:len()] = 0
    end
    return stdstrPtr
end

---@param stdstrPtr std_string
local function StdStringToStr(stdstrPtr)
	local stdstr = stdstrPtr[0]
    if stdstr.size >= 16 then
        return ffi.string(stdstr.data.buffer, stdstr.size)
    end
	return ffi.string(stdstr.data.sso_buffer, stdstr.size)
end

local ModDiskFileDeviceList = {}--挂载到表里面，防止被析构
local function ModDiskFileDevice(path)
    local mdfd = ffi.new("char[?]", 0x54)
    ModDiskFileDeviceList[#ModDiskFileDeviceList+1] = mdfd
    return cModDiskFileDevice(mdfd, ToStdString(""), path)
end

---将给定的创意工坊路径加载到指定的虚拟文件路径下
---<br>在后续的世界循环中挂载可能会崩溃，尽量在init里使用
---@param RealPathNoSlash string 不带 / 字符
---@param VirtualPath string
function ModFileAddVFS(RealPathNoSlash, VirtualPath)
    local path = ToStdString(RealPathNoSlash)
    local vpath = ToStdString(VirtualPath)
    cModFileAddVFS(
        UnknownVFSField,
        ModDiskFileDevice(path),
        1, --bool 开启了这个才能扫描绝对路径
        path,
        path,
        vpath,
		ffi.cast("char**", VFSystem)[0]
	)
end
