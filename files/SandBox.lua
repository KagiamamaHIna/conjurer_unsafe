---输入函数，返回沙盒环境下的函数和环境
---@param fn function
---@return function
---@return table env
local function NewSandBox(fn)
    local loadonce = {}
    local loaded = {}
    local env = {
        __loadonce = loadonce,
        __loaded = loaded,
    }
    env.do_mod_appends = function(filename)
        for _,v in ipairs(ModLuaFileGetAppends(filename) or {}) do
            env.dofile(v)
        end
    end

    env.dofile_once = function(filename)
        local result = nil
        local cached = loadonce[filename]
        if cached ~= nil then
            result = cached[1]
        else
            local f, err = loadfile(filename)
            if f == nil then return f, err end
            result = setfenv(f, env)()
            loadonce[filename] = { result }
            env.do_mod_appends(filename)
        end
        return result
    end

    env.dofile = function(filename)
        local f = loaded[filename]
        if f == nil then
            f, err = loadfile(filename)
            if f == nil then return f, err end
            loaded[filename] = setfenv(f, env)
        end
        local result = f()
        env.do_mod_appends(filename)
        return result
    end

    setmetatable(env, { __index = _G })

    return setfenv(fn, env), env
end

return NewSandBox
