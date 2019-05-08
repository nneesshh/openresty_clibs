#ifndef __LCH_H__
#define __LCH_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <lua.h>
#include <lauxlib.h>

LUALIB_API int luaopen_lcu(lua_State *L);

#ifdef __cplusplus
}
#endif

#endif /* __LCH_H__ */
