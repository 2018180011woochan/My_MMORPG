#pragma once
#ifndef __SESSIONMANAGER_H__
#define __SESSIONMANAGER_H__
#include "Session.h"

class SessionManager
{
public:
	// INIT
	int GetNewClientID();
	void InitBlock();
	void InitNpc();

public:
	// DB
	void InitDB();
	bool IsAllowAccess(int db_id, int cid);
	void SaveUserInfo(int db_id, int c_id);
	void ShowError(SQLHANDLE hHandle, SQLSMALLINT hType, RETCODE RetCode);

public:
	// NPC
	void MoveNPC(int _npc_id, int _c_id);
	void PathFinderPeace(int _npc_id, int _c_id);
	void PathFinderAgro(int _npc_id, int _c_id);
	void HitPlayer(int _n_id, int _p_id);

public:
	// USER
	void HitNPC(int _p_id, int n_id);	
	void CombatReward(int p_id, int n_id);

public:
	int Distance(int a, int b);
	int DistanceBlock(int a, int b);
	bool IsMovePossible(int _id, DIRECTION _direction);
	bool IsPeaceMonsterMovePossible(int _cid, int _mid, DIRECTION _direction);
	string NoticeAttack(int _attackID, int _targetID);

public:	
	array<Session, MAX_USER + NUM_NPC> clients;
	array<BLOCK, NUM_BLOCK> blocks;
	vector<int> ConnectedPlayer;

	//////////////// DB //////////////
	SQLHENV henv;
	SQLHDBC hdbc;
	SQLHSTMT hstmt = 0;
	SQLRETURN retcode;

	SQLINTEGER user_id, user_race, user_xpos, user_ypos, user_level, user_exp, user_hp, user_hpmax;
	SQLLEN cbuser_id = 0, cbrace = 0, cbuser_xpos = 0, cbuser_ypos = 0, cbuser_level,
		cbuser_exp, cbuser_hp, cbuser_hpmax;
	/// //////////////////////////////
};

extern SessionManager GSessionManager;

#endif // !__SESSIONMANAGER_H__
