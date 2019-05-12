#include "Offsets.h"

#define FindString(str, subject) if (subject.find(str) != std::string::npos)
#define Yield(L) lua_yield(L, 0);

//TODO: There is a glitch where it kicks you, once I rewrite it I will fix it

namespace LuaWrapper {
	int VCallHandler(lua_State* thread);
	int RCallHandler(DWORD RThread);

	LONG WINAPI CallCheck(PEXCEPTION_POINTERS ex)
	{
		switch (ex->ExceptionRecord->ExceptionCode)
		{
		case (DWORD)0x80000003L:
		{
			if (ex->ContextRecord->Eip == int3bp[1])
			{
				ex->ContextRecord->Eip = (DWORD)(RCallHandler);
				return EXCEPTION_CONTINUE_EXECUTION;
			}
			return -1;
		}
		default: return 0;
		}
		return 0;
	}

	DWORD locateINT3() {
		DWORD _s = ASLR(0x400000);
		const char i3_8opcode[8] = { 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC };
		for (int i = 0; i < MAX_INT; i++) {
			if (memcmp((void*)(_s + i), i3_8opcode, sizeof(i3_8opcode)) == 0) {
				return (_s + i);
			}
		}
		return NULL;
	}

	VOID CallCheckBypass()
	{
		int3bp.push_back(locateINT3());
		int3bp.push_back(locateINT3());
		AddVectoredExceptionHandler(1, CallCheck);
	}

	void Wrap(lua_State* L, DWORD rL, int index) {
		int Type = lua_type(L, index);
		std::cout << "Lua Type: " << Type << std::endl;
		switch (Type) {
		case LUA_TLIGHTUSERDATA:
			rlua_pushlightuserdata(rL, nullptr);
			break;
		case LUA_TNIL:
			rlua_pushnil(rL);
			break;
		case LUA_TNUMBER:
			rlua_pushnumber(rL, lua_tonumber(L, index));
			break;
		case LUA_TBOOLEAN:
			rlua_pushboolean(rL, lua_toboolean(L, index));
			break;
		case LUA_TSTRING:
			rlua_pushstring(rL, lua_tostring(L, index));
			break;
		case LUA_TTHREAD:
			rlua_newthread(rL);
			break;
		case LUA_TFUNCTION:
			lua_pushvalue(L, index);
			rlua_pushnumber(rL, luaL_ref(L, LUA_REGISTRYINDEX));
			rlua_pushcclosure(rL, int3bp[1], 1);
			break;
		case LUA_TTABLE: {
			lua_pushvalue(L, index);
			rlua_createtable(rL, 0, 0);
			lua_pushnil(L);
			while (lua_next(L, -2) != LUA_TNIL) {
				Wrap(L, rL, -2);
				Wrap(L, rL, -1);
				rlua_settable(rL, -3);
				lua_pop(L, 1);
			}
			lua_pop(L, 1);
			break;
			}
		case LUA_TUSERDATA: {
			rlua_rawgeti(rL, LUA_REGISTRYINDEX, *(int*)lua_topointer(L, index));
			break;
			}
		default: break;
		}
	}

	void UnWrap(DWORD rL, lua_State* L, int index) {
		int Type = rlua_type(rL, index);
		std::cout << "Roblox Type: " << Type << std::endl;
		switch (Type)
		{
		case RLUA_TLIGHTUSERDATA:
			lua_pushlightuserdata(L, nullptr);
			break;
		case RLUA_TNIL:
			lua_pushnil(L);
			break;
		case RLUA_TNUMBER:
			lua_pushnumber(L, rlua_tonumber(rL, index));
			break;
		case RLUA_TBOOLEAN:
			lua_pushboolean(L, rlua_toboolean(rL, index));
			break;
		case RLUA_TSTRING:
			lua_pushstring(L, rlua_tolstring(rL, index, 0));
			break;
		case RLUA_TTHREAD:
			lua_newthread(L);
			break;
		case RLUA_TFUNCTION:
			rlua_pushvalue(rL, index);
			lua_pushnumber(L, rluaL_ref(rL, LUA_REGISTRYINDEX));
			lua_pushcclosure(L, VCallHandler, 1);
			break;
		case RLUA_TTABLE:
			rlua_pushvalue(rL, index);
			lua_newtable(L);
			rlua_pushnil(rL);
			while (rlua_next(rL, -2) != RLUA_TNIL)
			{
				UnWrap(rL, L, -2);
				UnWrap(rL, L, -1);
				lua_settable(L, -3);
				rlua_pop(rL, 1);
			}
			rlua_pop(rL, 1);
			break;
		case RLUA_TUSERDATA: {
			int UD = (int)rlua_touserdata(rL, index);
			lua_rawgeti(L, LUA_REGISTRYINDEX, UD);
			if (!lua_isnil(L, -1))
				break;
			lua_settop(L, -2);
			rlua_pushvalue(rL, index);
			signed int userdatareference = rluaL_ref(rL, LUA_REGISTRYINDEX);
			int* userdata = (int*)lua_newuserdata(L, 1);
			*userdata = userdatareference;
			rlua_getmetatable(rL, index);
			UnWrap(rL, L, -1); 
			lua_setmetatable(L, -2);
			break;
		}
		default: break;
	}
}

	static int LuaResume(lua_State* thread)
	{
		lua_State* L = lua_tothread(thread, lua_upvalueindex(1));
		const int nargs = lua_gettop(thread);
		lua_xmove(thread, L, nargs);
		return lua_resume(L, nargs);
		lua_close(L);
	}

	int RCallHandler(DWORD RThread) {
		lua_State* thread = lua_newthread(VanillaState);

		lua_rawgeti(thread, LUA_REGISTRYINDEX, rlua_tonumber(RThread, lua_upvalueindex(1)));

		for (int n = 1; n <= rlua_gettop(RThread); ++n)
			UnWrap(RThread, thread, n);

		lua_pcall(thread, rlua_gettop(RThread), LUA_MULTRET, 0);

		int args = 0;

		for (int n = 1; n <= lua_gettop(thread); ++n, ++args) {
			Wrap(thread, RThread, n);
		}

		lua_settop(thread, 0);

		return args;
		lua_close(thread);
	}

	int VCallHandler(lua_State* thread) {
		auto rL = rlua_newthread(RobloxState);

		rlua_rawgeti(rL, LUA_REGISTRYINDEX, lua_tonumber(thread, lua_upvalueindex(1)));

		for (int n = 1; n <= lua_gettop(thread); ++n) {
			Wrap(thread, rL, n);
		}

		if (rlua_pcall(rL, lua_gettop(thread), LUA_MULTRET, 0))
		{
			std::string ErrM = rlua_tolstring(rL, -1, 0);

			FindString("yield", ErrM)
			{
				rlua_pop(rL, 1);
				lua_pushthread(thread);
				lua_pushcclosure(thread, &LuaResume, 1);
				Wrap(thread, rL, -1);
				return Yield(thread);
			}
			return 0;
		}

		int args = 0;

		for (int n = 1; n <= rlua_gettop(rL); ++n, ++args) {
			UnWrap(rL, thread, n);
		}

		rlua_settop(rL, 0);

		return args;
		lua_close(thread);
	}

}