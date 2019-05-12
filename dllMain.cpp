#include "Offsets.h"
#include <Windows.h>
#include <string>
#include <vector>
#include <iostream>
#include <iterator>
#include <Wininet.h>
#pragma comment(lib, "wininet.lib")
#include <stdio.h>
#pragma comment(lib,"ws2_32.lib")
#include "Utils.h"
#include "MinHook.h"

DWORD WINAPI PipeHandler(PVOID lvpParameter)
{
	std::string Data = "";
	HANDLE hPipe;
	char buffer[999999];
	DWORD dwRead;
	hPipe = CreateNamedPipe(TEXT("\\\\.\\pipe\\V3rmWrapperByRoboMat"),
		PIPE_ACCESS_DUPLEX | PIPE_TYPE_BYTE | PIPE_READMODE_BYTE,
		PIPE_WAIT,
		1,
		999999,
		999999,
		NMPWAIT_USE_DEFAULT_WAIT,
		NULL);
	while (hPipe != INVALID_HANDLE_VALUE)
	{
		if (ConnectNamedPipe(hPipe, NULL) != FALSE)
		{
			while (ReadFile(hPipe, buffer, sizeof(buffer) - 1, &dwRead, NULL) != FALSE)
			{
				buffer[dwRead] = '\0';
				try {
					try {
						Data = Data + buffer;
					}
					catch (...) {
					}
				}
				catch (std::exception e) {

				}
				catch (...) {

				}
			}
			
			//Runs Script
			Utils::RobloxInteractionBase::ExecuteScript(Data);
			//Runs Script

			Data = "";
		}
		DisconnectNamedPipe(hPipe);
	}
}



int MainInit() {
	Utils::Console::CreateConsole("V3rmWrapper | A Wrapper for V3rm By RoboMat");
	std::cout << "Note: This is not AtomWrapperX Source, It is a quick wrapper I made so that the community can learn from it" << std::endl;
	std::cout << "Note: I will be re-writing it soon" << std::endl;
	std::cout << "Scanning For ScriptContext" << std::endl;
	DWORD ScriptContextVFTable = ASLR(0x1CFEE1C);
	DWORD ScannedVFTable = (DWORD)(Utils::Scanner::ScanForScriptContext((char*)&ScriptContextVFTable));
	RobloxState = ScannedVFTable + 56 * 1 + 164 - *(DWORD *)(ScannedVFTable + 56 * 1 + 164);
	VanillaState = luaL_newstate();
	LuaWrapper::CallCheckBypass();
	luaL_openlibs(VanillaState);
	Sleep(1000);
	std::cout << "Wrapping Globals" << std::endl;
	Utils::RobloxInteractionBase::WrapGlobal(RobloxState, VanillaState, "print");
	Utils::RobloxInteractionBase::WrapGlobal(RobloxState, VanillaState, "game");
	//TODO: Add More Globals

	std::string Input;
	while (true) {
		std::getline(std::cin, Input);
		Utils::RobloxInteractionBase::ExecuteScript(Input);
	}
}

BOOL APIENTRY DllMain(HMODULE Module, DWORD Reason, void* Reserved)
{
	switch (Reason)
	{
	case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls(Module);
		CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)MainInit, NULL, NULL, NULL);
		break;
	case DLL_PROCESS_DETACH:
		break;
	default: break;
	}
	return TRUE;
}