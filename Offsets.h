#pragma once
#include "RetCheck.h"
#include <Windows.h>
#include <vector>
#include <sstream>
#include <fstream>
#include <istream>
#include <iterator>
#include <algorithm>
#include <string>
#include <windows.h> 
#include <iostream> 
#include <process.h>
#include <Shlwapi.h>
#include <ctime>
#include <cstdlib>
#include <chrono>
#include <unordered_map>

std::string DATASTREAM = "DataStream/WrapperInterval/DataHandler";

DWORD unprotect(DWORD addr)
{
	BYTE* tAddr = (BYTE*)addr;
	do
	{
		tAddr += 16;
	} while (!(tAddr[0] == 0x55 && tAddr[1] == 0x8B && tAddr[2] == 0xEC));

	DWORD funcSz = tAddr - (BYTE*)addr;

	PVOID nFunc = VirtualAlloc(NULL, funcSz, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	if (nFunc == NULL)
		return addr;

	memcpy(nFunc, (void*)addr, funcSz);

	BYTE* pos = (BYTE*)nFunc;
	BOOL valid = false;
	do
	{
		if (pos[0] == 0x72 && pos[2] == 0xA1 && pos[7] == 0x8B) {
			*(BYTE*)pos = 0xEB;

			DWORD cByte = (DWORD)nFunc;
			do
			{
				if (*(BYTE*)cByte == 0xE8)
				{
					DWORD oFuncPos = addr + (cByte - (DWORD)nFunc);
					DWORD oFuncAddr = (oFuncPos + *(DWORD*)(oFuncPos + 1)) + 5;

					if (oFuncAddr % 16 == 0)
					{
						DWORD relativeAddr = oFuncAddr - cByte - 5;
						*(DWORD*)(cByte + 1) = relativeAddr;

						cByte += 4;
					}
				}

				cByte += 1;
			} while (cByte - (DWORD)nFunc < funcSz);

			valid = true;
		}
		pos += 1;
	} while ((DWORD)pos < (DWORD)nFunc + funcSz);

	if (!valid)
	{
		VirtualFree(nFunc, funcSz, MEM_RELEASE);
		return addr;
	}

	return (DWORD)nFunc;
}

#define ASLR(x) (x - 0x400000 + (DWORD)GetModuleHandleA(0))
#define Declare(address, returnValue, callingConvention, ...) (returnValue(callingConvention*)(__VA_ARGS__))(unprotect(ASLR(address)))
#define rlua_gettop(rL) ((*(DWORD *)(rL + 20) - *(DWORD *)(rL + 24)) >> 4)
#define rlua_pop(rL, n) rlua_settop(rL, -(n) - 1)
#define rlua_getglobal(rL, k) rlua_getfield(rL, -10002, k)
#define RLUA_TNIL						 0
#define RLUA_TLIGHTUSERDATA              1
#define RLUA_TNUMBER					 2
#define RLUA_TBOOLEAN					 3
#define RLUA_TSTRING					 4
#define RLUA_TTHREAD                     5
#define RLUA_TFUNCTION					 6
#define RLUA_TTABLE						 7
#define RLUA_TUSERDATA					 8
#define RLUA_TPROTO					     9
#define RLUA_TUPVAL					     10

extern "C" {
#include "Lua\lua.h"
#include "Lua\lua.hpp"
#include "Lua\lualib.h"
#include "Lua\lauxlib.h"
#include "Lua\luaconf.h"
#include "Lua\llimits.h"
#include "Lua/ldo.h"
}

#include <vector>

DWORD RobloxState;
lua_State* VanillaState;
std::vector<int> int3bp;

union r_Value {
	PVOID gc;
	PVOID p;
	double n;
	int b;
};


struct r_TValue {
	r_Value value;
	int tt;
};

struct Userdata {
	void* UD;
	int Key;
	int Key2;
};

// -- Lua C Functions -- \\

auto rlua_index2adr = Declare(0x8068d0, r_TValue*, __stdcall, DWORD a1, int idx);
auto rlua_getfield = Declare(0x80f7f0, void, __stdcall, int a1, int idx, const char *key);
auto rlua_gettable = Declare(0x80fc70, void, __cdecl, int a1, int idx);
auto rlua_createtable = Declare(0x80f480, void, __cdecl, int a1, int narray, int nrec);
auto rlua_tonumber = Declare(0x811d90, double, __stdcall, int a1, signed int a2);
auto rlua_touserdata = Declare(0x811ed0, void*, __cdecl, int a1, int idx);
auto rlua_newuserdata = Declare(0x810350, void*, __cdecl, int a1, int size);
auto rlua_getmetatable = Declare(0x80fac0, int, __cdecl, int a1, signed int a2);
auto rlua_toboolean = Declare(0x811b90, int, __cdecl, int a1, signed int a2);
auto rlua_pushcclosure = Declare(0x8106e0, void, __fastcall, DWORD a1, int a2, int a3);
auto rlua_pushlightuserdata = Declare(0x810950, void, __cdecl, DWORD a1, void *p);
auto rlua_pushstring = Declare(0x810b70, int, __cdecl, DWORD a1, const char* a2);
auto rlua_pushvalue = Declare(0x810c30, int, __cdecl, DWORD a1, int idx);
auto rlua_pushnumber = Declare(0x810ae0, void, __stdcall, DWORD a1, double a2);
auto rlua_pushboolean = Declare(0x810660, void, __cdecl, DWORD a1, int a2);
auto rlua_pushnil = Declare(0x810a70, void, __cdecl, DWORD a1);
auto rlua_next = Declare(0x810410, int, __cdecl, DWORD a1, int idx);
auto rlua_type = Declare(0x811f00, int, __cdecl, DWORD a1, int idx);
auto rlua_setfield = Declare(0x8115c0, void, __fastcall, DWORD a1, int a2, const char* a3);
auto rlua_settable = Declare(0x811980, void, __cdecl, DWORD a1, int idx);
auto rlua_tolstring = Declare(0x811c50, const char*, __fastcall, DWORD a1, int a2, std::size_t* a3);
auto rlua_setreadonly = Declare(0x8118a0, void, __cdecl, DWORD a1, int a2, int a3);
auto rlua_setmetatable = Declare(0x811770, int, __cdecl, DWORD a1, int a2);
auto rlua_newthread = Declare(0x810260, int, __cdecl, DWORD a1);
auto rlua_rawgeti = Declare(0x810ee0, void, __cdecl, DWORD a1, int idx, int n);
auto rlua_rawseti = Declare(0x811060, void, __cdecl, DWORD a1, int idx, int n);
auto rlua_settop = Declare(0x811a10, void, __cdecl, DWORD a1, int idx);
auto rlua_getfenv = Declare(0x80f700, void, __cdecl, DWORD a1, int idx);
auto rlua_pcall = Declare(0x810590, int, __cdecl, DWORD a1, int a2, int a3, int a4);
auto rluaL_ref = Declare(0x80a310, int, __cdecl, DWORD a1, int a2);
auto rluaL_error = Declare(0x809cf0, int, __cdecl, DWORD a1, const char *lol, ...);
auto writerf = Declare(0x542240, int, __cdecl, int idx, const char *lol, ...);
auto Spawn = Declare(0x7f03e0, int, __cdecl, DWORD a1);
//DWORD ScriptContextAddr = ASLR(0x1CFEE1C);


#define rlua_pushcfunction(L,f)	rlua_pushcclosure(L, (f), 0)

void rlua_pushobject(int rL, r_TValue *Value) {
	r_TValue *Top = (r_TValue*)*(DWORD *)(rL + 32);
	Top->tt = Value->tt;
	Top->value = Value->value;
	*(DWORD *)(rL + 32) += 16;
}




