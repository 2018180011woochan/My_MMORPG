#include "pch.h"
#include "Session.h"
#include "SocketUtils.h"

Session::Session()
{
	_socket = SocketUtils::CreateSocket();
}

Session::~Session()
{
	SocketUtils::Close(_socket);
}

void Session::Disconnect(const WCHAR* cause)
{
}

HANDLE Session::GetHandle()
{
	return reinterpret_cast<HANDLE>(_socket);
}

void Session::Dispatch(IocpEvent* iocpEvent, int32 numOfByte)
{
	// ������ ���ú곪 ���尰���� ���⼭ �Ѵ�
}

void Session::RegisterConnect()
{
}

void Session::RegisterRecv()
{
}

void Session::RegisterSend()
{
}

void Session::ProcessConnect()
{
}

void Session::ProcessRecv(int32 nomOfBytes)
{
}

void Session::ProcessSend(int32 nomOfBytes)
{
}

void Session::HandleError(int32 errorCode)
{
}
