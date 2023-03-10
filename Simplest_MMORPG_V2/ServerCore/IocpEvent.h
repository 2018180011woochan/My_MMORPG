#pragma once

class Session;

enum class EventType : uint8
{
	Connect,
	Disconnect,
	Accept,
	Recv,
	Send
};

// OverlappedEx ??????
class IocpEvent : public OVERLAPPED
{
public:
	IocpEvent(EventType eventType);

	void Init();

public:
	EventType eventType;
	IocpObjectRef owner;
};

// ConnectEvent
class ConnectEvent : public IocpEvent
{
public:
	ConnectEvent() : IocpEvent(EventType::Connect) { }
};

// DisConnectEvent
class DisconnectEvent : public IocpEvent
{
public:
	DisconnectEvent() : IocpEvent(EventType::Disconnect) { }
};

// AcceptEvent
class AcceptEvent : public IocpEvent
{
public:
	AcceptEvent() : IocpEvent(EventType::Accept) { }

public:
	SessionRef session = nullptr;
};

// RecvEvent
class RecvEvent : public IocpEvent
{
public:
	RecvEvent() : IocpEvent(EventType::Recv) { }
};

// SendEvent
class SendEvent : public IocpEvent
{
public:
	SendEvent() : IocpEvent(EventType::Send) { }

	vector<SendBufferRef> sendBuffers;
};
