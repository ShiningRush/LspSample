// dllmain.cpp : ���� DLL Ӧ�ó������ڵ㡣
#include "stdafx.h"

TCHAR	g_szCurrentApp[MAX_PATH];	// ��ǰ���ñ�DLL�ĳ��������
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
			// ȡ����ģ�������
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
