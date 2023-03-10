#pragma once
#include "Session.h"



class GameSession : public PacketSession
{
public:
	~GameSession()
	{
		cout << "~ServerSession" << endl;
	}
	virtual void OnConnected() override;
	virtual void OnDisconnected() override;
	virtual void OnRecvPacket(BYTE* buffer, int32 len) override;
	virtual void OnSend(int32 len) override;

public:
	int ClientID = -1;
	SESSION_STATE session_state = ST_FREE;
};

