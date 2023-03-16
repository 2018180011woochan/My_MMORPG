#include "pch.h"
#include "../Server/protocol.h"
#include "GameSessionManager.h"
#include "GameSession.h"

GameSessionManager GSessionManager;
bool ConnectedControl = true;

void GameSessionManager::Add(GameSessionRef session)
{
	WRITE_LOCK;
	if (ConnectedControl) {
		session->session_state = ST_ACCEPTED;
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

int GameSessionManager::GetAcceptedID()
{
	for (int i = 0; i < MAX_USER; ++i)
	{
		if (_sessions[i]->ClientID == -1) {
			_sessions[i]->ClientID = i;
			return i;
		}
	}
	return 0;
}
