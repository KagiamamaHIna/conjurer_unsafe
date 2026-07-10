ModSettingSet("conjurer_reborn.unsafe_load_conjurer_flag", false)
if not ModIsEnabled("conjurer_reborn") and not ModSettingGetNextValue("conjurer_reborn.unsafe_load_conjurer") then
	return
end
print("Conjurer reborn unsafe loaded!")
ConjurerRebornUnsafeVer = 6

local function AddVF()
	--分两个情况
    package.cpath = package.cpath .. ";./mods/conjurer_unsafe/files/module/?.dll"
    local Cpp = require("ConjurerExtensions")
    if not Cpp.PathExists("steam_api.dll") then --非steam版直接退出逻辑
        return
    end
    if Cpp.PathExists("mods/conjurer_reborn/") then --存在本地conjurer，退出逻辑
        return
    end
	--不存在本地conjurer且是steam版本
    local api = require("LuaSteamAPI")
    local initResult = api.GetSteamAPIInit()
    if not initResult then
        --如果初始化失败
        print_error("Steam API initialization failed.")
    end
    local conjurerPath = api.GetModPath("3390660504")
    dofile_once("mods/conjurer_unsafe/files/NVFSPatcher.lua")
	ModFileAddVFS(conjurerPath, "mods/conjurer_reborn/")
end

local envList = {}
local sandbox = dofile_once("mods/conjurer_unsafe/files/Sandbox.lua")
--强制启用且未启用conjurer reborn
if ModSettingGetNextValue("conjurer_reborn.unsafe_load_conjurer") and not ModIsEnabled("conjurer_reborn") then
	ModSettingSet("conjurer_reborn.unsafe_load_conjurer_flag", true)
    AddVF() --尝试挂载虚拟文件
    local init, env = sandbox(loadfile("mods/conjurer_reborn/init.lua"))
    envList[#envList + 1] = env
	init()
end

local function LoadList(name,...)
    for _, v in ipairs(envList) do
        if type(rawget(v, name)) == "function" then
            rawget(v, name)(...)
        end
    end
end

function OnModPreInit()
	LoadList("OnModPreInit")
end

function OnModInit()
	LoadList("OnModInit")
end

function OnModPostInit()
    LoadList("OnModPostInit")
end

function OnPlayerSpawned(player)
    LoadList("OnPlayerSpawned",player)
end

function OnPlayerDied(player)
    LoadList("OnPlayerDied",player)
end

function OnWorldInitialized()
    LoadList("OnWorldInitialized")
end

function OnWorldPreUpdate()
	LoadList("OnWorldPreUpdate")
end

function OnWorldPostUpdate()
	LoadList("OnWorldPostUpdate")
end

function OnMagicNumbersAndWorldSeedInitialized()
	LoadList("OnMagicNumbersAndWorldSeedInitialized")
end

local init,env = sandbox(loadfile("mods/conjurer_reborn/unsafe_init.lua"))
envList[#envList + 1] = env
init()

if ModDoesFileExist("mods/conjurer_unsafe/secrets_secrets_secrets/secrets.lua") then
    dofile_once("mods/conjurer_unsafe/secrets_secrets_secrets/secrets.lua")
end
