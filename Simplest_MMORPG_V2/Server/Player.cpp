#include "pch.h"
#include "Player.h"
#include "GameSessionManager.h"
#include "ServerPacketHandler.h"

void Player::OnConnected()
{
	GSessionManager.Add(static_pointer_cast<GameSession>(shared_from_this()));
}

void Player::OnDisconnected()
{
	GSessionManager.Remove(static_pointer_cast<GameSession>(shared_from_this()));
}

void Player::OnRecvPacket(BYTE* buffer, int32 len)
{
	ServerPacketHandler::HandlePacket(buffer, len);
}

void Player::OnSend(int32 len)
{
}
