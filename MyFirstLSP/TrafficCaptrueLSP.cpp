//////////////////////////////////////////////////
// LSP.cpp�ļ�


// ����Ҫʹ��UNICODE�ַ���
#pragma once
#define UNICODE
#define _UNICODE



#include "stdafx.h"
#include "Debug.h"

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Iphlpapi.lib")


#define CONFIGURATION_KEY TEXT("SOFTWARE\\Wow6432Node\\YiBan\\HSPS")

WSPUPCALLTABLE g_pUpCallTable;		// �ϲ㺯���б����LSP�������Լ���α�������ʹ����������б�
WSPPROC_TABLE g_NextProcTable;		// �²㺯���б�
extern TCHAR	g_szCurrentApp[MAX_PATH];	// ��ǰ���ñ�DLL�ĳ��������
TCHAR *hspsRegPath = NULL;
bool isInit = false;


LPWSAPROTOCOL_INFOW GetProvider(LPINT lpnTotalProtocols)
{
	DWORD dwSize = 0;
	int nError;
	LPWSAPROTOCOL_INFOW pProtoInfo = NULL;
	
	// ȡ����Ҫ�ĳ���
	if(::WSCEnumProtocols(NULL, pProtoInfo, &dwSize, &nError) == SOCKET_ERROR)
	{
		if(nError != WSAENOBUFS)
			return NULL;
	}
	
	pProtoInfo = (LPWSAPROTOCOL_INFOW)::GlobalAlloc(GPTR, dwSize);
	*lpnTotalProtocols = ::WSCEnumProtocols(NULL, pProtoInfo, &dwSize, &nError);
	return pProtoInfo;
}

void FreeProvider(LPWSAPROTOCOL_INFOW pProtoInfo)
{
	::GlobalFree(pProtoInfo);
}


bool IsConfigApp()
{
	// �ж������ļ��е�·���Ƿ�Ϊִ�н��̵�·��
	bool result = false;
	TCHAR configFilePath[256], *p;
	::GetFullPathName(_tcscat(hspsRegPath, L"\\Config\\lspPassApp.config") , 256, configFilePath, &p);
	std::ifstream fin(configFilePath);
	if (!fin) {
		ODS(L"AppConfigFile open error!\n");
	}
	else
	{
		char authorizedAppPath[256];
		while (!fin.eof())            //�ж��ļ��Ƿ������
		{
			fin.getline(authorizedAppPath, 256);
			wchar_t wcsStr[256];
			swprintf(wcsStr, L"%S", authorizedAppPath);
			if (_tcsstr(g_szCurrentApp, wcsStr))
			{
				result = true;
				break;
			}
		}

	}

	fin.close();

	return result;
}

bool IsHSPSApp()
{
	if (!isInit)
	{
		HKEY newKey;
		DWORD lpDataType, lpDataLength;
		BYTE lpHspsRegPathData[256];
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
				lpHspsRegPathData,
				&lpDataLength);

			if (ERROR_SUCCESS == lresult)
			{
				hspsRegPath = (TCHAR*)lpHspsRegPathData;
				isInit = true;
			}
		}
	}

	if (hspsRegPath != NULL)
	{
		TCHAR *result = _tcsstr(g_szCurrentApp, hspsRegPath);
		if (result != NULL)
		{
			return true;
		}


		if (IsConfigApp())
		{
			return true;
		}
	}

	return false;
}

bool IsInNicConfigFile(char* checkingNicName)
{
	// �ж������ļ��е���������
	bool result = false;
	TCHAR configFilePath[256], *p;
	::GetFullPathName(_tcscat(hspsRegPath, L"\\Config\\networkInterfaceCard.config"), 256, configFilePath, &p);
	std::ifstream fin(configFilePath);
	if (!fin) {
		ODS(L"NicConfigFile open error!\n");
	}
	else
	{
		char nicName[256];
		while (!fin.eof())            //�ж��ļ��Ƿ������
		{
			fin.getline(nicName, 256);
			if (strstr(nicName, checkingNicName))
			{
				result = true;
				break;
			}
		}

	}

	fin.close();

	return result;
}

bool Is4GNetworkInterfaceCard(DWORD checkIp)
{
	PIP_ADAPTER_INFO pAdapterInfo = NULL;
	ULONG ulLen = 0;
	DWORD hspsNICAddr = 0;

	// Ϊ�������ṹ�����ڴ�
	::GetAdaptersInfo(pAdapterInfo, &ulLen);
	pAdapterInfo = (PIP_ADAPTER_INFO)::GlobalAlloc(GPTR, ulLen);

	// ȡ�ñ����������ṹ��Ϣ
	if (::GetAdaptersInfo(pAdapterInfo, &ulLen) == ERROR_SUCCESS)
	{
		while (pAdapterInfo != NULL)
		{

			if (pAdapterInfo ->Type == MIB_IF_TYPE_PPP )
			{
				hspsNICAddr = ::inet_addr(pAdapterInfo->IpAddressList.IpAddress.String);
				break;
			}

			pAdapterInfo = pAdapterInfo->Next;
		}
	}

	return hspsNICAddr == checkIp;
}

void PrintSocketInfo(SOCKET s)
{
	struct sockaddr_in sa;
	int len = sizeof(sa);
	if (!getsockname(s, (struct sockaddr *)&sa, &len)) 
	{
		if (Is4GNetworkInterfaceCard(sa.sin_addr.S_un.S_addr))
		{
			ODS(L"����4G����");
		}
		else
		{
			ODS(L"������������");
		}
	}
}


bool IsCanConnectNetwork(SOCKET s)
{
	ODS(L"��ʼ���socket��ip");
	struct sockaddr_in sa;
	int len = sizeof(sa);
	if (!getsockname(s, (struct sockaddr *)&sa, &len))
	{
		ODS(L"��ȡ��socket��ip ����ʼ����Ƿ�4G����");
		if (Is4GNetworkInterfaceCard(sa.sin_addr.S_un.S_addr))
		{
			ODS(L"����4G��������ʼ����Ƿ������������");
			return IsHSPSApp();
		}
	}

	return true;
}

int WSPAPI WSPSendTo(
	SOCKET			s,
	LPWSABUF		lpBuffers,
	DWORD			dwBufferCount,
	LPDWORD			lpNumberOfBytesSent,
	DWORD			dwFlags,
	const struct sockaddr FAR * lpTo,
	int				iTolen,
	LPWSAOVERLAPPED	lpOverlapped,
	LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine,
	LPWSATHREADID	lpThreadId,
	LPINT			lpErrno
)
{
	ODS1(L" query send to... %s", g_szCurrentApp);
	//PrintSocketInfo(s);

	//// �ܾ�����Ŀ�Ķ˿�Ϊ4567��UDP���
	//SOCKADDR_IN sa = *(SOCKADDR_IN*)lpTo;
	//if(sa.sin_port == htons(4567) || true)
	//{
	//	int	iError;
	//	g_NextProcTable.lpWSPShutdown(s, SD_BOTH, &iError);
	//	*lpErrno = WSAECONNABORTED;

	//	ODS(L" deny a sendto by sendTo ");
	//	return SOCKET_ERROR;
	//}

	if (!IsCanConnectNetwork(s))
	{
		int	iError;
		g_NextProcTable.lpWSPShutdown(s, SD_BOTH, &iError);
		*lpErrno = WSAECONNABORTED;
		return SOCKET_ERROR;
	}


	return g_NextProcTable.lpWSPSendTo(s, lpBuffers, dwBufferCount, lpNumberOfBytesSent, dwFlags, lpTo
			, iTolen, lpOverlapped, lpCompletionRoutine, lpThreadId, lpErrno);

}

INT
WSPAPI
WSPSend(
	IN SOCKET s,
	IN LPWSABUF lpBuffers,
	IN DWORD dwBufferCount,
	IN LPDWORD lpNumberOfBytesSent,
	IN DWORD dwFlags,
	IN LPWSAOVERLAPPED lpOverlapped,
	IN LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine,
	IN LPWSATHREADID lpThreadId,
	OUT INT FAR *lpErrno
)
/*++
Routine Description:

Send data on a connected socket.

Arguments:

s                   - A descriptor identifying a connected socket.

lpBuffers           - A  pointer  to  an  array of WSABUF structures.  Each
WSABUF  structure  contains a pointer to a buffer and
the length of the buffer.

dwBufferCount       - The  number  of  WSABUF  structures  in the lpBuffers
array.

lpNumberOfBytesSent - A pointer to the number of bytes sent by this call.

dwFlags             - Flags.

lpOverlapped        - A pointer to a WSAOVERLAPPED structure.

lpCompletionRoutine - A  pointer  to the completion routine called when the
send operation has been completed.

lpThreadId          - A  pointer to a thread ID structure to be used by the
provider in a subsequent call to WPUQueueApc().

lpErrno             - A pointer to the error code.

Return Value:

If  no  error  occurs  and  the  send  operation has completed immediately,
WSPSend() returns the number of bytes received.  If the connection has been
closed,  it  returns  0.  Note that in this case the completion routine, if
specified, will   have  already  been  queued.   Otherwise, a  value  of
SOCKET_ERROR  is  returned, and  a  specific  error  code  is available in
lpErrno.   The  error  code  WSA_IO_PENDING  indicates  that the overlapped
operation  has  been  successfully  initiated  and  that completion will be
indicated  at  a  later  time.   Any  other  error  code  indicates that no
overlapped operation was initiated and no completion indication will occur.

--*/
{
	//int	iError;
	//g_NextProcTable.lpWSPShutdown(s, SD_BOTH, &iError);
	//*lpErrno = WSAECONNABORTED;

	ODS1(L" query send ... %s", g_szCurrentApp);
	if (!IsCanConnectNetwork(s))
	{
		int	iError;
		g_NextProcTable.lpWSPShutdown(s, SD_BOTH, &iError);
		*lpErrno = WSAECONNABORTED;
		return SOCKET_ERROR;
	}

	return g_NextProcTable.lpWSPSend(s, lpBuffers, dwBufferCount, lpNumberOfBytesSent, dwFlags, lpOverlapped
		, lpCompletionRoutine, lpThreadId, lpErrno);
}

INT
WSPAPI
WSPConnect(
	IN SOCKET s,
	IN const struct sockaddr FAR *name,
	IN INT namelen,
	IN LPWSABUF lpCallerData,
	IN LPWSABUF lpCalleeData,
	IN LPQOS lpSQOS,
	IN LPQOS lpGQOS,
	OUT INT FAR *lpErrno
)
/*++
Routine Description:

Establish a connection to a peer,
exchange connect data,
and specify needed
quality of service based on the supplied flow spec.

Arguments:

s            - A descriptor identifying an unconnected socket.

name         - The name of the peer to which the socket is to be connected.

namelen      - The length of the name.

lpCallerData - A  pointer to the user data that is to be transferred to the
peer during connection established.

lpCalleeData - A pointer to a buffer into which may be copied any user data
received from the peer during connection establishment.

lpSQOS       - A  pointer  to  the  flow  specs  for socket s, one for each
direction.

lpGQOS       - A  pointer  to  the  flow  specs  for  the  socket group (if
applicable).

lpErrno      - A pointer to the error code.

Return Value:

If  no  error  occurs, WSPConnect()  returns NO_ERROR.  Otherwise, it
returns SOCKET_ERROR, and a specific erro rcode is available in lpErrno.

--*/
{
	if (!IsCanConnectNetwork(s))
	{
		return SOCKET_ERROR;
	}

	return g_NextProcTable.lpWSPConnect(s, name, namelen, lpCallerData, lpCalleeData, lpSQOS
		, lpGQOS, lpErrno);

}

int WSPAPI WSPStartup(
  WORD wVersionRequested,
  LPWSPDATA lpWSPData,
  LPWSAPROTOCOL_INFO lpProtocolInfo,
  WSPUPCALLTABLE UpcallTable,
  LPWSPPROC_TABLE lpProcTable
)
{
	ODS1(L"  WSPStartup...  %s \n", g_szCurrentApp);
	
	if(lpProtocolInfo->ProtocolChain.ChainLen <= 1)
	{	
		return WSAEPROVIDERFAILEDINIT;
	}
	
	// �������ϵ��õĺ�����ָ�루�������ǲ�ʹ������
	g_pUpCallTable = UpcallTable;

	// ö��Э�飬�ҵ��²�Э���WSAPROTOCOL_INFOW�ṹ	
	WSAPROTOCOL_INFOW	NextProtocolInfo;
	int nTotalProtos, i;
	LPWSAPROTOCOL_INFOW pProtoInfo = GetProvider(&nTotalProtos);
	// �²����ID	
	DWORD dwBaseEntryId = lpProtocolInfo->ProtocolChain.ChainEntries[1];
	for(i=0; i<nTotalProtos; i++)
	{
		if(pProtoInfo[i].dwCatalogEntryId == dwBaseEntryId)
		{
			memcpy(&NextProtocolInfo, &pProtoInfo[i], sizeof(NextProtocolInfo));
			break;
		}
	}
	if(i >= nTotalProtos)
	{
		ODS(L" WSPStartup:	Can not find underlying protocol \n");
		return WSAEPROVIDERFAILEDINIT;
	}

	// �����²�Э���DLL
	int nError;
	TCHAR szBaseProviderDll[MAX_PATH];
	int nLen = MAX_PATH;
	// ȡ���²��ṩ����DLL·��
	if(::WSCGetProviderPath(&NextProtocolInfo.ProviderId, szBaseProviderDll, &nLen, &nError) == SOCKET_ERROR)
	{
		ODS1(L" WSPStartup: WSCGetProviderPath() failed %d \n", nError);
		return WSAEPROVIDERFAILEDINIT;
	}
	if(!::ExpandEnvironmentStrings(szBaseProviderDll, szBaseProviderDll, MAX_PATH))
	{
		ODS1(L" WSPStartup:  ExpandEnvironmentStrings() failed %d \n", ::GetLastError());
		return WSAEPROVIDERFAILEDINIT;
	}
	// �����²��ṩ����
	HMODULE hModule = ::LoadLibrary(szBaseProviderDll);
	if(hModule == NULL)
	{
		ODS1(L" WSPStartup:  LoadLibrary() failed %d \n", ::GetLastError());
		return WSAEPROVIDERFAILEDINIT;
	}

	// �����²��ṩ�����WSPStartup����
	LPWSPSTARTUP  pfnWSPStartup = NULL;
	pfnWSPStartup = (LPWSPSTARTUP)::GetProcAddress(hModule, "WSPStartup");
	if(pfnWSPStartup == NULL)
	{
		ODS1(L" WSPStartup:  GetProcAddress() failed %d \n", ::GetLastError());
		return WSAEPROVIDERFAILEDINIT;
	}

	// �����²��ṩ�����WSPStartup����
	LPWSAPROTOCOL_INFOW pInfo = lpProtocolInfo;
	if(NextProtocolInfo.ProtocolChain.ChainLen == BASE_PROTOCOL)
		pInfo = &NextProtocolInfo;

	int nRet = pfnWSPStartup(wVersionRequested, lpWSPData, pInfo, UpcallTable, lpProcTable);
	if(nRet != ERROR_SUCCESS)
	{
		ODS1(L" WSPStartup:  underlying provider's WSPStartup() failed %d \n", nRet);
		return nRet;
	}

	// �����²��ṩ�ߵĺ�����
	g_NextProcTable = *lpProcTable;

	// �޸Ĵ��ݸ��ϲ�ĺ�����Hook����Ȥ�ĺ�����������Ϊʾ������Hook��WSPSendTo����
	// ��������Hook������������WSPSocket��WSPCloseSocket��WSPConnect��
	lpProcTable->lpWSPSendTo = WSPSendTo;
	lpProcTable->lpWSPSend = WSPSend;
	lpProcTable->lpWSPConnect = WSPConnect;

	FreeProvider(pProtoInfo);
	return nRet;
}




