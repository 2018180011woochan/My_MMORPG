#pragma once
#ifndef __SESSIONMANAGER_H__
#define __SESSIONMANAGER_H__
#include "Session.h"

class SessionManager
{
public:
	// INIT
	int get_new_client_id();
	void Init_Block();
	void Init_npc();

public:
	// DB
	void initialize_DB();
	bool isAllowAccess(int db_id, int cid);
	void Save_UserInfo(int db_id, int c_id);
	void ShowError(SQLHANDLE hHandle, SQLSMALLINT hType, RETCODE RetCode);

public:
	// NPC
	void Move_NPC(int _npc_id, int _c_id);
	void PathFinder_Peace(int _npc_id, int _c_id);
	void PathFinder_Agro(int _npc_id, int _c_id);
	void Hit_Player(int _n_id, int _p_id);

public:
	// USER
	void Hit_NPC(int _p_id, int n_id);	
	void Combat_Reward(int p_id, int n_id);

public:
	SECTOR GetSector(int _race, int _id);
	void SetSector(int _race, int _id);

public:
	int distance(int a, int b);
	int distance_block(int a, int b);
	bool isMovePossible(int _id, DIRECTION _direction);
	bool isPeaceMonsterMovePossible(int _cid, int _mid, DIRECTION _direction);
	string Notice_Attack(int _attackID, int _targetID);

public:	
	array<Session, MAX_USER + NUM_NPC> clients;
	array<BLOCK, NUM_BLOCK> blocks;

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
