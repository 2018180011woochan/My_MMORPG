#pragma once

class GameSession;

using GameSessionRef = shared_ptr<GameSession>;
class GameSessionManager
{
public:
	void Add(GameSessionRef session);
	void Remove(GameSessionRef session);
	void Broadcast(SendBufferRef sendBuffer);
	void Send(int id, SendBufferRef sendBuffer);

public:
	short GetTargetX(int id);
	short GetTargetY(int id);
	void SetTargetPos(int id, int x, int y);

public:
	int GetAcceptedID();

private:
	USE_LOCK;
	vector<GameSessionRef> _sessions;
};

extern GameSessionManager GSessionManager;
