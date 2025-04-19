if not ModIsEnabled("conjurer_reborn") then
	return
end
print("Conjurer reborn unsafe loaded!")
ConjurerRebornUnsafeVer = 5

dofile_once("mods/conjurer_reborn/unsafe_init.lua")

if ModDoesFileExist("mods/conjurer_unsafe/secrets_secrets_secrets/secrets.lua") then
	dofile_once("mods/conjurer_unsafe/secrets_secrets_secrets/secrets.lua")
end
