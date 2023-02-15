#pragma once
#include "NetworkAddress.h"

class SocketUtils
{
public:
	static LPFN_CONNECTEX		ConnectEx;
	static LPFN_DISCONNECTEX	DisConnectEx;
	static LPFN_ACCEPTEX		AcceptEx;

public:
	static void Init();
	static void Clear();

	static bool BindWindowsFunction(SOCKET socket, GUID guid, LPVOID* fn);
	static SOCKET CreateSocket();		// 임의의 tcp소켓을 만드는 함수
};

