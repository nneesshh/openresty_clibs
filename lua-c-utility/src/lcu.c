#include "lcu.h"

#include <stdlib.h>
#include <assert.h>

#include "lcu_platform.h"
#include "lcu_netinfo.h"
#include "memory_stream.h"

#include "lcu_platform.h"

#if defined(LCU_PLATFORM_WIN32) || defined(LCU_PLATFORM_WINRT)
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#elif defined(LCU_PLATFORM_LINUX) || defined(LCU_PLATFORM_ANDROID) || defined(LCU_PLATFORM_DARWIN) 
#include <netdb.h>
#include <unistd.h>
#endif

#if !defined(LUA_VERSION_NUM) || LUA_VERSION_NUM < 502

#define lua_getuservalue     lua_getfenv
#define lua_setuservalue     lua_setfenv

#define lua_rawlen           lua_objlen

#define luaL_setfuncs(L,f,n) luaL_register(L,NULL,f)

/* Compatibility for Lua 5.1 and older LuaJIT.
 *
 * compat_luaL_setfuncs() is used to create a module table where the functions
 * have upvalues. Code borrowed from Lua 5.2 source's luaL_setfuncs().
 */
static void 
compat_luaL_setfuncs(lua_State *L, const luaL_Reg *reg, int nup)
{
	int i;

	luaL_checkstack(L, nup, "too many upvalues");
	for (; reg->name != NULL; reg++) {  /* fill the table with given functions */
		for (i = 0; i < nup; i++)  /* copy upvalues to the top */
			lua_pushvalue(L, -nup);
		lua_pushcclosure(L, reg->func, nup);  /* closure with those upvalues */
		lua_setfield(L, -(nup + 2), reg->name);
	}
	lua_pop(L, nup);  /* remove upvalues */
}
#else
#define compat_luaL_setfuncs(L, reg, nup) luaL_setfuncs(L, reg, nup)
#endif

static int
l_get_platform(lua_State *L)
{
	int platform = get_platform();
	lua_pushinteger(L, platform);
	return 1;
}

static int
l_get_platform_name(lua_State *L)
{
	const char *platform_name = get_platform_name();
	lua_pushstring(L, platform_name);
	return 1;
}

static int
l_get_native_endian(lua_State *L)
{
	int endian = get_native_endian();
	lua_pushinteger(L, endian);
	return 1;
}

static int
l_get_net_info(lua_State *L)
{
	int num;
	netinfo_t *netinfo_list = get_netinfo(&num);
	if (num > 0) {
		netinfo_t *net_info = netinfo_list;
		lua_pushstring(L, net_info->name); // netcard name
		lua_pushstring(L, net_info->ip); // ip
		lua_pushstring(L, net_info->mac); // mac

		static char hostname[256] = { 0 };
		int r = gethostname(hostname, sizeof(hostname) - 1);
		if (0 == r) {
			lua_pushstring(L, hostname); // hostname
			return 4;
		}
		return 3;
	}

	lua_pushnil(L);
	lua_pushstring(L, "unknown error");
	return 2;
}

/* memory stream */
#define STREAM_BUFFER_DEFAULT_SIZE 4096

#define MEMORY_STREAM_META "lcu.mt.memory_stream"
#define check_memory_stream(L, idx) \
    (memory_stream_t*) luaL_checkudata(L, idx, MEMORY_STREAM_META)

static int
l_create_memory_stream(lua_State *L)
{
	int sz = STREAM_BUFFER_DEFAULT_SIZE;
	if (lua_isnumber(L, 1)) {
		sz = luaL_checkint(L, 1);
	}

	void *u = lua_newuserdata(L, sizeof(memory_stream_t));
	if (!u) {
		lua_pushstring(L, "create memory stream failed");
		lua_error(L);
		return 0;
	}

	init_memory_stream((memory_stream_t *)u, sz);
	luaL_getmetatable(L, MEMORY_STREAM_META);
	lua_setmetatable(L, -2);
	return 1;
}

static int
l_memory_stream_gc(lua_State *L)
{
	memory_stream_t *p = check_memory_stream(L, 1);
	if (p) {
		free(p->buf);
	}
	return 0;
}

static int
l_memory_stream_reset(lua_State *L)
{
	memory_stream_t *p = check_memory_stream(L, 1);
	memory_stream_reset(p);
	return 0;
}

static int
l_memory_stream_rewind(lua_State *L)
{
	memory_stream_t *p = check_memory_stream(L, 1);
	memory_stream_rewind(p);
	return 0;
}

static int
l_memory_stream_skip(lua_State *L)
{
	memory_stream_t *p = check_memory_stream(L, 1);
	int len = 0;
	if (lua_isnumber(L, 2)) {
		len = luaL_checkint(L, 2);
	}
	memory_stream_skip(p, len);
	return 0;
}

static int
l_memory_stream_skip_all(lua_State *L)
{
	memory_stream_t *p = check_memory_stream(L, 1);
	memory_stream_skip_all(p);
	return 0;
}

static int
l_memory_stream_get_used_size(lua_State *L)
{
	memory_stream_t *p = check_memory_stream(L, 1);
	size_t sz = memory_stream_get_used_size(p);
	lua_pushinteger(L, sz);
	return 1;
}

static int
l_memory_stream_get_free_size(lua_State *L)
{
	memory_stream_t *p = check_memory_stream(L, 1);
	size_t sz = memory_stream_get_free_size(p);
	lua_pushinteger(L, sz);
	return 1;
}

static int
l_memory_stream_ensure_free_size(lua_State *L)
{
	memory_stream_t *p = check_memory_stream(L, 1);
	int size = luaL_checkint(L, 2);
	bool b = memory_stream_ensure_free_size(p, size);
	lua_pushboolean(L, b);
	return 1;
}

static int
l_memory_stream_read_byte(lua_State *L)
{
	memory_stream_t *p = check_memory_stream(L, 1);
	int8_t d = memory_stream_read_byte(p);
	lua_pushinteger(L, d);
	return 1;
}

static int
l_memory_stream_read_int16(lua_State *L)
{
	memory_stream_t *p = check_memory_stream(L, 1);
	int16_t d = memory_stream_read_int16(p);
	lua_pushinteger(L, d);
	return 1;
}

static int
l_memory_stream_read_int32(lua_State *L)
{
	memory_stream_t *p = check_memory_stream(L, 1);
	int32_t d = memory_stream_read_int32(p);
	lua_pushinteger(L, d);
	return 1;
}

static int
l_memory_stream_read_int64(lua_State *L)
{
	memory_stream_t *p = check_memory_stream(L, 1);
	int64_t d = memory_stream_read_int64(p);
	lua_pushnumber(L, (lua_Number)d);
	return 1;
}

static int
l_memory_stream_read_string(lua_State *L)
{
	memory_stream_t *p = check_memory_stream(L, 1);
	uint16_t len;
	const char *str = memory_stream_read_string(p, &len);
	lua_pushlstring(L, str, len);
	return 1;
}

static int
l_memory_stream_read_raw(lua_State *L)
{
	memory_stream_t *p = check_memory_stream(L, 1);
	/* read all if no arg */
	int len = memory_stream_get_used_size(p);
	if (lua_isnumber(L, 2)) {
		lua_Integer _in_len = luaL_checkint(L, 2);
		len = (_in_len > len) ? len : _in_len;
	}
	lua_pushlstring(L, p->cursor_r, len);
	memory_stream_skip(p, len);
	return 1;
}

static int
l_memory_stream_write_byte(lua_State *L)
{
	memory_stream_t *p = check_memory_stream(L, 1);
	int8_t d = 0;
	int type = lua_type(L, 2);
	if (LUA_TNUMBER == type) {
		d = (int8_t)lua_tointeger(L, 2);
	}
	else if (LUA_TSTRING == type) {
		size_t len;
		const char *str = lua_tolstring(L, 2, &len);
		if (len > 0)
			d = *str;
	}
	memory_stream_write_byte(p, d);
	return 0;
}

static int
l_memory_stream_write_int16(lua_State *L)
{
	memory_stream_t *p = check_memory_stream(L, 1);
	int16_t d = luaL_checkint(L, 2);
	memory_stream_write_int16(p, d);
	return 0;
}

static int
l_memory_stream_write_int32(lua_State *L)
{
	memory_stream_t *p = check_memory_stream(L, 1);
	int32_t d = luaL_checkint(L, 2);
	memory_stream_write_int32(p, d);
	return 0;
}

static int
l_memory_stream_write_int64(lua_State *L)
{
	memory_stream_t *p = check_memory_stream(L, 1);
	int64_t d = luaL_checklong(L, 2);
	memory_stream_write_int64(p, d);
	return 0;
}

static int
l_memory_stream_write_string(lua_State *L)
{
	memory_stream_t *p = check_memory_stream(L, 1);
	size_t len;
	const char *str = luaL_checklstring(L, 2, &len);
	memory_stream_write_string(p, str, (int16_t)len);
	return 0;
}

static int
l_memory_stream_write_raw(lua_State *L)
{
	memory_stream_t *p = check_memory_stream(L, 1);
	size_t len;
	const char *str = luaL_checklstring(L, 2, &len);
	memory_stream_write_raw(p, (unsigned char *)str, len);
	return 0;
}

static const struct luaL_Reg _lua_c_utility[] = {
	{ "get_platform", l_get_platform },
	{ "get_platform_name", l_get_platform_name },
	{ "get_native_endian", l_get_native_endian },
	{ "get_net_info", l_get_net_info },
	{ "create_memory_stream", l_create_memory_stream },
	{ NULL, NULL }
};


LUALIB_API int
luaopen_lcu(lua_State *L)
{
	int top = lua_gettop(L);

	/* memory stream meta */
	luaL_Reg reg_memory_stream[] = {
		{ "__gc", l_memory_stream_gc },
		{ "reset", l_memory_stream_reset },
		{ "rewind", l_memory_stream_rewind },
		{ "skip", l_memory_stream_skip },
		{ "skip_all", l_memory_stream_skip_all },
		{ "get_used_size", l_memory_stream_get_used_size },
		{ "get_free_size", l_memory_stream_get_free_size },
		{ "ensure_free_size", l_memory_stream_ensure_free_size },
		{ "read_byte", l_memory_stream_read_byte },
		{ "read_int16", l_memory_stream_read_int16 },
		{ "read_int32", l_memory_stream_read_int32 },
		{ "read_int64", l_memory_stream_read_int64 },
		{ "read_string", l_memory_stream_read_string },
		{ "read_raw", l_memory_stream_read_raw },
		{ "write_byte", l_memory_stream_write_byte },
		{ "write_int16", l_memory_stream_write_int16 },
		{ "write_int32", l_memory_stream_write_int32 },
		{ "write_int64", l_memory_stream_write_int64 },
		{ "write_string", l_memory_stream_write_string },
		{ "write_raw", l_memory_stream_write_raw },
		{ NULL, NULL }
	};
	if (luaL_newmetatable(L, MEMORY_STREAM_META)) {
		luaL_setfuncs(L, reg_memory_stream, 0);
		lua_pushvalue(L, -1);
		lua_setfield(L, -2, "__index");
		lua_pop(L, 1);  /* pop MEMORY_STREAM_META */
	}

	/* lcu */
	lua_newtable(L);
	compat_luaL_setfuncs(L, _lua_c_utility, 0);

	assert(1 == lua_gettop(L) - top);
	// return 1;

	lua_setglobal(L, "lcu");
	assert(0 == lua_gettop(L) - top);
	return 0;
}