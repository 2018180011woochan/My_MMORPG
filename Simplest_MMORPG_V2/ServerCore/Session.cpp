#include "pch.h"
#include "Session.h"
#include "SocketUtils.h"
#include "Service.h"

Session::Session() : _recvBuffer(BUFFER_SIZE)
{
	_socket = SocketUtils::CreateSocket();
}

Session::~Session()
{
	SocketUtils::Close(_socket);
}

void Session::Send(BYTE* buffer, int32 len)
{
	// temp
	SendEvent* sendEvent = new SendEvent();
	sendEvent->owner = shared_from_this();
	sendEvent->buffer.resize(len);
	::memcpy(sendEvent->buffer.data(), buffer, len);

	WRITE_LOCK;
	RegisterSend(sendEvent);
}

bool Session::Connect()
{
	// �������� ������ ���� ���涧 
	// ������ �̿��Ͽ� ���� ������ �پ�� �Ѵ�

	return RegisterConnect();
}

void Session::Disconnect(const WCHAR* cause)
{
	if (_connected.exchange(false) == false)
		return;

	wcout << "Disconnect : " << cause << endl;

	OnDisconnected();	// ������ �ڵ忡�� ������
	GetService()->ReleaseSession(GetSessionRef());

	RegisterDisconnect();
}

HANDLE Session::GetHandle()
{
	return reinterpret_cast<HANDLE>(_socket);
}

void Session::Dispatch(IocpEvent* iocpEvent, int32 numOfByte)
{
	// ������ ���ú곪 ���尰���� ���⼭ �Ѵ�
	switch (iocpEvent->eventType)
	{
	case EventType::Connect:
		ProcessConnect();
		break;
	case EventType::Disconnect:
		ProcessDisconnect();
		break;
	case EventType::Recv:
		ProcessRecv(numOfByte);
		break;
	case EventType::Send:
		ProcessSend(static_cast<SendEvent*>(iocpEvent), numOfByte);
		break;
	default:
		break;
	}
}

bool Session::RegisterConnect()
{
	if (IsConnected())
		return false;

	if (GetService()->GetServiceType() != ServiceType::Client)
		return false;

	if (SocketUtils::SetReuseAddress(_socket, true) == false)
		return false;

	if (SocketUtils::BindAnyAddress(_socket, 0/*���°�*/) == false)
		return false;

	_connectEvent.Init();
	_connectEvent.owner = shared_from_this();

	DWORD numOfByte = 0;
	SOCKADDR_IN sockAddr = GetService()->GetNetAddress().GetSockAddr();
	if (false == SocketUtils::ConnectEx(_socket, reinterpret_cast<SOCKADDR*>(&sockAddr),
		sizeof(sockAddr), nullptr, 0, &numOfByte, &_connectEvent))
	{
		int32 errorCode = ::WSAGetLastError();
		if (errorCode != WSA_IO_PENDING)
		{
			_connectEvent.owner = nullptr;
			return false;
		}
	}
	return true;
}

bool Session::RegisterDisconnect()
{
	_disConnectEvent.Init();
	_disConnectEvent.owner = shared_from_this();

	if (false == SocketUtils::DisConnectEx(_socket, &_disConnectEvent, TF_REUSE_SOCKET, 0))
	{
		int32 errorCode = ::WSAGetLastError();
		if (errorCode != WSA_IO_PENDING)
		{
			_disConnectEvent.owner = nullptr;
			return false;
		}
	}
	return true;
}

void Session::RegisterRecv()
{
	if (IsConnected() == false)
		return;

	_recvEvent.Init();
	_recvEvent.owner = shared_from_this();

	WSABUF wsaBuf;
	wsaBuf.buf = reinterpret_cast<char*>(_recvBuffer.WritePos());
	wsaBuf.len = _recvBuffer.FreeSize();

	DWORD numOfBytes = 0;
	DWORD flags = 0;
	if (SOCKET_ERROR == ::WSARecv(_socket, &wsaBuf, 1, &numOfBytes, &flags, &_recvEvent, nullptr))
	{
		int32 errorCode = ::WSAGetLastError();
		if (errorCode != WSA_IO_PENDING)
		{
			HandleError(errorCode);
			cout << "Recv Error" << endl;
			_recvEvent.owner = nullptr;
		}
	}
}

void Session::RegisterSend(SendEvent* sendEvent)
{
	if (IsConnected() == false)
		return;

	WSABUF wsaBuf;
	wsaBuf.buf = (char*)sendEvent->buffer.data();
	wsaBuf.len = (ULONG)sendEvent->buffer.size();

	DWORD numOfByte = 0;
	if (SOCKET_ERROR == ::WSASend(_socket, &wsaBuf, 1, &numOfByte, 0, sendEvent, nullptr))
	{
		int32 errorCode = ::WSAGetLastError();
		if (errorCode != WSA_IO_PENDING)
		{
			HandleError(errorCode);
			sendEvent->owner = nullptr;
			delete sendEvent;
		}
	}
}

void Session::ProcessConnect()
{
	_connectEvent.owner = nullptr;

	_connected.store(true);

	// ���� ���
	GetService()->AddSession(GetSessionRef());

	// ������ �ڵ忡�� �����ε�
	OnConnected();

	// ���� ���
	RegisterRecv();
}

void Session::ProcessDisconnect()
{
	_disConnectEvent.owner = nullptr;
}

void Session::ProcessRecv(int32 numOfBytes)
{
	_recvEvent.owner = nullptr;

	if (numOfBytes == 0)
	{
		Disconnect(L"Rev 0");
		return;
	}

	if (_recvBuffer.OnWrite(numOfBytes) == false)
	{
		Disconnect(L"Onwrite Overflow");
		return;
	}

	int32 dataSize = _recvBuffer.DataSize();
	// processLen ���� ó���� �������� ũ��
	int32 processLen = OnRecv(_recvBuffer.ReadPos(), dataSize);
	if (processLen < 0 || dataSize < processLen || _recvBuffer.OnRead(processLen) == false)
	{
		Disconnect(L"Onread Overflow");
		return;
	}

	// Ŀ�� ����
	_recvBuffer.Clean();

	// ���� ��� (���˴븦 �ٽ� ����)
	RegisterRecv();
}

void Session::ProcessSend(SendEvent* sendEvent, int32 numOfBytes)
{
	sendEvent->owner = nullptr;
	delete sendEvent;

	if (numOfBytes == 0)
	{
		Disconnect(L"Send 0");
		return;
	}

	// ������ �ڵ忡�� �����ε�
	OnSend(numOfBytes);
}

void Session::HandleError(int32 errorCode)
{
	switch (errorCode)
	{
	case WSAECONNRESET:
	case WSAECONNABORTED:
		Disconnect(L"HanddleError");
		break;
	default:
		cout << "Handle Error :" << errorCode << endl;
		break;
	}
}
