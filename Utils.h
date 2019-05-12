#include "Wrapper.h"

namespace Utils {
	namespace Scanner {
		//This is not my scanner, I don't scan in my personal wrappers but hook gettop. But for the sake of this wrapper ill scan.
		bool Compare(const char* pData, const char* bMask, const char* szMask) {
			while (*szMask) {
				if (*szMask != '?') {
					if (*pData != *bMask) {
						return 0;
					};
				};
				++szMask, ++pData, ++bMask;
			};
			return 1;
		};


		DWORD ScanForScriptContext(const char* ScriptContextVFTableOffsetted) {
			MEMORY_BASIC_INFORMATION MemoryBasicInformation = { 0 };
			SYSTEM_INFO SystemInfo = { 0 };
			GetSystemInfo(&SystemInfo);
			DWORD Start = (DWORD)SystemInfo.lpMinimumApplicationAddress;
			DWORD End = (DWORD)SystemInfo.lpMaximumApplicationAddress;
			do {
				while (VirtualQuery((void*)Start, &MemoryBasicInformation, sizeof(MemoryBasicInformation))) {
					if ((MemoryBasicInformation.Protect & PAGE_READWRITE) && !(MemoryBasicInformation.Protect & PAGE_GUARD)) {
						for (DWORD i = (DWORD)(MemoryBasicInformation.BaseAddress); i - (DWORD)(MemoryBasicInformation.BaseAddress) < MemoryBasicInformation.RegionSize; ++i) {
							if (Compare((const char*)i, ScriptContextVFTableOffsetted, "xxxx"))
								return i;
						};
					};
					Start += MemoryBasicInformation.RegionSize;
				};
			} while (Start < End);
			return 0;
		};
	}
	namespace RobloxInteractionBase {
		void WrapGlobal(DWORD RState, lua_State* VState, const char* global) {
			rlua_getglobal(RState, global);
			LuaWrapper::UnWrap(RState, VState, -1);
			lua_setglobal(VState, global);
		}
		void ExecuteScript(std::string Script) {
			luaL_dostring(VanillaState, Script.c_str());
		}

		int ChangeIdentity(DWORD state, int identityvalue) {
			try{
			DWORD* baseOffset = (DWORD *)(state - 40);
			*baseOffset ^= (identityvalue ^ (unsigned __int8)*baseOffset) & 0x1F;
			}
			catch (int e) {
				std::cout << "There was a error while setting the identity to 6" << std::endl;
			}
			return 1;
		}
	}
	namespace Console {
		boolean __stdcall CreateConsole(std::string name)
		{
			DWORD nOldProtect;
			if (!VirtualProtect(FreeConsole, 1, PAGE_EXECUTE_READWRITE, &nOldProtect)){ return FALSE; }
			*(BYTE*)(FreeConsole) = 0xC3;
			if (!VirtualProtect(FreeConsole, 1, nOldProtect, &nOldProtect)){ return FALSE; }
			AllocConsole();
			freopen("CONOUT$", "w", stdout);
			freopen("CONIN$", "r", stdin);
			HWND ConsoleHandle = GetConsoleWindow();
			SetWindowPos(ConsoleHandle, HWND_TOPMOST, 50, 20, 0, 0, SWP_DRAWFRAME | SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
			ShowWindow(ConsoleHandle, 1);
			SetConsoleTitle(name.c_str());
		}
	}
}