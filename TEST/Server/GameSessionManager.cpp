#include "pch.h"
#include "../Server/protocol.h"
#include "GameSessionManager.h"
#include "GameSession.h"
#include "ServerPacketHandler.h"

GameSessionManager GSessionManager;
bool ConnectedControl = true;

void GameSessionManager::Add(GameSessionRef session)
{
	WRITE_LOCK;
	if (ConnectedControl) {
		session->session_state = ST_INGAME;
		_sessions.push_back(session);

		ConnectedControl = false;
	}
	else
		ConnectedControl = true;

}

void GameSessionManager::Remove(GameSessionRef session)
{
	WRITE_LOCK;

	//_sessions.erase(_sessions.begin() + session->ClientID);
	
}

void GameSessionManager::Broadcast(SendBufferRef sendBuffer)
{
	WRITE_LOCK;
	for (GameSessionRef session : _sessions)
	{
		session->Send(sendBuffer);
	}
}

void GameSessionManager::Send(int id, SendBufferRef sendBuffer)
{
	WRITE_LOCK;
	for (GameSessionRef session : _sessions)
	{
		if (id == session->ClientID)
			session->Send(sendBuffer);
	}
}

short GameSessionManager::GetTargetX(int id)
{
	WRITE_LOCK;
	for (auto& p : _sessions)
	{
		if (p->ClientID == id)	
			return p->x;
	}
	return -1;
}

short GameSessionManager::GetTargetY(int id)
{
	WRITE_LOCK;
	for (auto& p : _sessions)
	{
		if (p->ClientID == id)
			return p->y;
	}
	return -1;
}

void GameSessionManager::SetTargetPos(int id, int x, int y)
{
	WRITE_LOCK;
	for (auto& p : _sessions)
	{
		if (p->ClientID == id)
		{
			p->x = x;
			p->y = y;
		}
	}
}

int GameSessionManager::GetAcceptedID()
{
	WRITE_LOCK;
	for (int i = 0; i < MAX_USER; ++i)
	{
		if (_sessions[i]->ClientID == -1) {
			_sessions[i]->ClientID = i;
			return i;
		}
	}
	return 0;
}
