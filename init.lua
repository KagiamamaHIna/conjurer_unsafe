if not ModIsEnabled("conjurer_reborn") then
	return
end
print("Conjurer reborn unsafe loaded!")
ConjurerRebornUnsafeVer = 3

dofile_once("mods/conjurer_reborn/unsafe_init.lua")
