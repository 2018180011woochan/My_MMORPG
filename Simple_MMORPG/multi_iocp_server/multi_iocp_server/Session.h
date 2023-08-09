#pragma once
#ifndef __SESSION_H_
#define __SESSION_H_

#include "Include.h"
#include "OVER_EXP.h"

class Session {
	OVER_EXP _RecvOver;

public:
	// lock
	mutex	_Lock;
	mutex	_ViewListLock;

	SESSION_STATE _SessionState;
	SOCKET _Socket;
	OBJ_STAT _ObjStat;
	unordered_set <int> _ViewList;
	unordered_set <int> _ViewListBlock;

	//vector<int> _MyParty;
	unordered_set <int> _MyParty;

	ATTACKTYPE _AttackType;
	MOVETYPE _MoveType;
	int		_PrevRemainTime;

public:
	Session()
	{
		_ObjStat.ID = -1;
		_Socket = 0;
		_ObjStat.x = rand() % W_WIDTH;
		_ObjStat.y = rand() % W_HEIGHT;
		_ObjStat.Name[0] = 0;
		_ObjStat.Level = 1;
		_ObjStat.Exp = 0;
		_ObjStat.MaxExp = _ObjStat.Level * 100;
		_ObjStat.MaxHP = _ObjStat.Level * 100;
		_ObjStat.HP = _ObjStat.MaxHP;
		_ObjStat.Race = RACE::RACE_END;
		_SessionState = ST_FREE;
	}

	~Session() {}

	void DoRecv();
	void DoSend(void* packet);

	// Packet
	void SendLoginOkPacket(int c_id);
	void SendMovePacket(int c_id, int client_time);
	void SendAddObjectPacket(int c_id);
	void SendAddBlockPacket(int _id);
	void SendChatPacket(int c_id, const char* mess);
	void SendRemovePacket(int c_id);
	void SendRemoveBlockPacket(int _id);
	void SendStatChangePacket(int c_id, int n_id);
	void SendPlayerAttackPacket(int c_id);
	void SendNoticePacket(char _message[BUF_SIZE]);
};


#endif // !__SESSION_H_
