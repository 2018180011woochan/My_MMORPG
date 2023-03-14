#include "pch.h"
#include "ServerSession.h"
#include "ClientPacketHandler.h"

void ServerSession::OnConnected()
{
	SendBufferRef sendBuffer = ClientPacketHandler::Make_CS_LOGIN(L"Èûµé¾î¿ä");

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
