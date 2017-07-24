// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "stdafx.h"

TCHAR	g_szCurrentApp[MAX_PATH];	// 当前调用本DLL的程序的名称
HMODULE g_hModule;

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		{
			// 取得主模块的名称
			::GetModuleFileName(NULL, g_szCurrentApp, MAX_PATH);
			g_hModule = hModule;
		}
	break;
	}
	return TRUE;

	//switch (ul_reason_for_call)
	//{
	//case DLL_PROCESS_ATTACH:
	//case DLL_THREAD_ATTACH:
	//case DLL_THREAD_DETACH:
	//case DLL_PROCESS_DETACH:
	//	break;
	//}
	//return TRUE;
}
