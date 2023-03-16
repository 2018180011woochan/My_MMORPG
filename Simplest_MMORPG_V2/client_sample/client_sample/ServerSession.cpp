#include "pch.h"
#include "ServerSession.h"
#include "ClientPacketHandler.h"

PacketSessionRef mysession;

void ServerSession::OnConnected()
{
	mysession = GetPacketSessionRef();

	SendBufferRef sendBuffer = ClientPacketHandler::Make_CS_LOGIN(L"¿ìÂù");

	Send(sendBuffer);
}

void ServerSession::OnDisconnected()
{
}

void ServerSession::OnRecvPacket(BYTE* buffer, int32 len)
{
	ClientPacketHandler::HandlePacket(buffer, len);
}

void ServerSession::OnSend(int32 len)
{
}
