// MyDemoARP.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include "InitSock.h"
#include <fstream>

CInitSock initSock;
#define CONFIGURATION_KEY TEXT("SOFTWARE\\Wow6432Node\\YiBan\\HSPS")
#define UNICODE
#define _UNICODE

void PrintLocalIPAddress()
{
	char szHost[256];
	// ȡ�ñ�����������
	int error = ::gethostname(szHost, 256);
	// ͨ���������õ���ַ��Ϣ
	hostent *pHost = ::gethostbyname(szHost);
	// ��ӡ������IP��ַ
	in_addr addr;
	for (int i = 0; ; i++)
	{
		char *p = pHost->h_addr_list[i];
		if (p == NULL)
			break;

		memcpy(&addr.S_un.S_addr, p, pHost->h_length);
		char *szIp = ::inet_ntoa(addr);
		wchar_t *temp = L"D:\\Program Files\\��������\\HSPS\\Test.exe";
		wchar_t wcsStr[100];
		swprintf(wcsStr, 100, L" ����IP��ַ��%S  \n ", szIp);
		char *result = strstr((char*)wcsStr, (char*)temp);
		printf("%S ", wcsStr);
	}
}

void GetRegValue()
{
	TCHAR configFilePath[256], *p;
	::GetFullPathName(L"D:\\Program Files\\��������\\HSPS\\lspPassApp.config", 256, configFilePath, &p);
	std::ifstream fin("D:\\Program Files\\��������\\HSPS\\Config\\lspPassApp.config");
	if (!fin) {
		printf( "File open error!\n");
		return;
	}
	char c[256];
	while (!fin.eof())            //�ж��ļ��Ƿ������
	{
		fin.getline(c, 80);
		printf("OK!\n");
	}
	fin.close();

	HKEY newKey;
	DWORD lpDataType, lpDataLength;
	BYTE lpData[100];
	LSTATUS lresult = RegOpenKeyEx(
		HKEY_LOCAL_MACHINE,
		CONFIGURATION_KEY,
		0,
		KEY_ALL_ACCESS,
		&newKey);
	if (ERROR_SUCCESS == lresult) {
		lresult = RegQueryValueEx(
			newKey,
			L"InstallPath",
			0,
			&lpDataType,
			lpData,
			&lpDataLength);

		if (ERROR_SUCCESS == lresult)
		{
			TCHAR *temp = L"D:\\Program Files\\��������\\HSPS\\Test.exe";
			TCHAR *result = _tcsstr(temp, (TCHAR*)lpData);
			scanf_s("%d");
		}
	}
}

void GetAdapterInfos()
{
	u_char	g_ucLocalMac[6];	// ����MAC��ַ
	DWORD	g_dwGatewayIP;		// ����IP��ַ
	DWORD	g_dwLocalIP;		// ����IP��ַ
	DWORD	g_dwMask;			// ��������

	PIP_ADAPTER_INFO pAdapterInfo = NULL;
	ULONG ulLen = 0;

	// Ϊ�������ṹ�����ڴ�
	::GetAdaptersInfo(pAdapterInfo, &ulLen);
	pAdapterInfo = (PIP_ADAPTER_INFO)::GlobalAlloc(GPTR, ulLen);

	// ȡ�ñ����������ṹ��Ϣ
	if (::GetAdaptersInfo(pAdapterInfo, &ulLen) == ERROR_SUCCESS)
	{
		while (pAdapterInfo != NULL)
		{
			memcpy(g_ucLocalMac, pAdapterInfo->Address, 6);
			g_dwGatewayIP = ::inet_addr(pAdapterInfo->GatewayList.IpAddress.String);
			g_dwLocalIP = ::inet_addr(pAdapterInfo->IpAddressList.IpAddress.String);
			g_dwMask = ::inet_addr(pAdapterInfo->IpAddressList.IpMask.String);

			if (strstr(pAdapterInfo->Description, "ZTE Wireless Ethernet Adapter #2"))
			{
				printf(" \n -------------------- ����������Ϣ -----------------------\n\n");
				in_addr in;
				in.S_un.S_addr = g_dwLocalIP;
				printf("      IP Address : %s \n", ::inet_ntoa(in));

				in.S_un.S_addr = g_dwMask;
				printf("     Subnet Mask : %s \n", ::inet_ntoa(in));

				in.S_un.S_addr = g_dwGatewayIP;
				printf(" Default Gateway : %s \n", ::inet_ntoa(in));

				u_char *p = g_ucLocalMac;
				printf("     MAC Address : %02X-%02X-%02X-%02X-%02X-%02X \n", p[0], p[1], p[2], p[3], p[4], p[5]);

				printf(" \n \n ");
			}

			pAdapterInfo = pAdapterInfo->Next;
		}
	}

}

int main()
{
	GetAdapterInfos();

	scanf_s("%d");
    return 0;
}