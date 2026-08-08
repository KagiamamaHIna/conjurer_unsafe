local ffi = require("ffi")
--通用的std string声明
pcall(ffi.cdef, [[
union ssoUnion {
    char* buffer;
    char sso_buffer[16];
};

struct std_string { /* VC++ std::string */
    union ssoUnion data;
    size_t size;
    size_t capacity;
};
]])
