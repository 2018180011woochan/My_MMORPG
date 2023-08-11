#include "pch.h"
#include "SessionManager.h"
#include "SectorManager.h"

SessionManager GSessionManager;

int SessionManager::GetNewClientID()
{
	for (int i = 0; i < MAX_USER; ++i) {
		clients[i]._Lock.lock();
		if (clients[i]._SessionState == ST_FREE) {
			clients[i]._SessionState = ST_ACCEPTED;
			clients[i]._Lock.unlock();
			return i;
		}
		clients[i]._Lock.unlock();
	}
	return -1;
}

void SessionManager::InitNpc()
{
	for (int i = MAX_USER; i < NUM_NPC + MAX_USER; ++i)
		clients[i]._ObjStat.ID = i;
	cout << "NPC initialize Begin.\n";

	for (int i = MAX_USER; i < MAX_USER + 50000; ++i)
	{
		// Skeleton
		clients[i]._SessionState = ST_INGAME;
		clients[i]._ObjStat.Race = RACE::RACE_SKELETON;
		clients[i]._ObjStat.Level = 1;
		clients[i]._ObjStat.MaxHP = clients[i]._ObjStat.Level * 100;
		clients[i]._ObjStat.HP = clients[i]._ObjStat.MaxHP;
		clients[i]._MoveType = MOVETYPE::MOVETYPE_FIX;
		clients[i]._AttackType = ATTACKTYPE::ATTACKTYPE_PEACE;
		strcpy_s(clients[i]._ObjStat.Name, "Skeleton");

		GSectorManager.PushSector(i);
		//SetSector(RACE_SKELETON, i);
	}
	for (int i = MAX_USER + 50000; i < MAX_USER + 100000; ++i)
	{
		// Wraith
		clients[i]._SessionState = ST_INGAME;
		clients[i]._ObjStat.Race = RACE::RACE_WRIATH;
		clients[i]._ObjStat.Level = 2;
		clients[i]._ObjStat.MaxHP = clients[i]._ObjStat.Level * 100;
		clients[i]._ObjStat.HP = clients[i]._ObjStat.MaxHP;
		clients[i]._MoveType = MOVETYPE::MOVETYPE_FIX;
		clients[i]._AttackType = ATTACKTYPE::ATTACKTYPE_AGRO;
		clients[i]._ObjStat.targetID = -1;
		strcpy_s(clients[i]._ObjStat.Name, "Wriath");

		GSectorManager.PushSector(i);
		//SetSector(RACE_WRIATH, i);
	}
	for (int i = MAX_USER + 100000; i < MAX_USER + NUM_NPC; ++i)
	{
		// Devil
		clients[i]._SessionState = ST_INGAME;
		clients[i]._ObjStat.Race = RACE::RACE_DEVIL;
		clients[i]._ObjStat.Level = 3;
		clients[i]._ObjStat.MaxHP = clients[i]._ObjStat.Level * 100;
		clients[i]._ObjStat.HP = clients[i]._ObjStat.MaxHP;
		clients[i]._MoveType = MOVETYPE::MOVETYPE_ROAMING;
		clients[i]._AttackType = ATTACKTYPE::ATTACKTYPE_PEACE;
		strcpy_s(clients[i]._ObjStat.Name, "Devil");

		GSectorManager.PushSector(i);
		//SetSector(RACE_DEVIL, i);
	}

	cout << "NPC Initialization complete.\n";
}

void SessionManager::InitBlock()
{
	for (int i = 0; i < NUM_BLOCK; ++i)
	{
		blocks[i].BlockID = i;
		blocks[i].x = rand() % W_WIDTH;
		blocks[i].y = rand() % W_WIDTH;
	}
}

void SessionManager::InitDB()
{
	setlocale(LC_ALL, "korean");

	retcode = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv);
	retcode = SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER*)SQL_OV_ODBC3, 0);
	retcode = SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc);
	SQLSetConnectAttr(hdbc, SQL_LOGIN_TIMEOUT, (SQLPOINTER)5, 0);

	retcode = SQLConnect(hdbc, (SQLWCHAR*)L"project_db_odbc", SQL_NTS, (SQLWCHAR*)NULL, 0, NULL, 0);

	retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);

	std::cout << "DB Access OK\n";
}

bool SessionManager::IsAllowAccess(int db_id, int cid)
{
	retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);

	wstring _db_id = to_wstring(db_id);

	wstring storedProcedure = L"EXEC select_info ";
	storedProcedure += _db_id;

	retcode = SQLExecDirect(hstmt, (SQLWCHAR*)storedProcedure.c_str(), SQL_NTS);
	ShowError(hstmt, SQL_HANDLE_STMT, retcode);

	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {

		// Bind columns 1, 2, and 3  
		retcode = SQLBindCol(hstmt, 1, SQL_C_LONG, &user_race, 10, &cbrace);
		retcode = SQLBindCol(hstmt, 2, SQL_C_LONG, &user_xpos, 10, &cbuser_xpos);
		retcode = SQLBindCol(hstmt, 3, SQL_C_LONG, &user_ypos, 10, &cbuser_ypos);
		retcode = SQLBindCol(hstmt, 4, SQL_C_LONG, &user_level, 10, &cbuser_level);
		retcode = SQLBindCol(hstmt, 5, SQL_C_LONG, &user_exp, 10, &cbuser_exp);
		retcode = SQLBindCol(hstmt, 6, SQL_C_LONG, &user_hp, 10, &cbuser_hp);
		retcode = SQLBindCol(hstmt, 7, SQL_C_LONG, &user_hpmax, 10, &cbuser_hpmax);

		// Fetch and print each row of data. On an error, display a message and exit.  
		for (int i = 0; ; i++) {
			retcode = SQLFetch(hstmt);
			if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
			{
				////// DB에서 정보 받아오기 //////////		
				clients[cid]._ObjStat.Race = user_race;
				clients[cid]._ObjStat.x = user_xpos;
				clients[cid]._ObjStat.y = user_ypos;
				clients[cid]._ObjStat.Level = user_level;
				clients[cid]._ObjStat.Exp = user_exp;
				clients[cid]._ObjStat.HP = user_hp;
				clients[cid]._ObjStat.MaxHP = user_hpmax;
				return true;
			}
			else
			{
				std::cout << "DB에 정보가 없어 추가하였습니다\n";
				return false;
			}
		}
	}
}

void SessionManager::SaveUserInfo(int db_id, int c_id)
{
	retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);

	wstring mydb_id = to_wstring(clients[c_id]._ObjStat.DBID);
	wstring race = to_wstring(clients[c_id]._ObjStat.Race);
	wstring xpos = to_wstring(clients[c_id]._ObjStat.x);
	wstring ypos = to_wstring(clients[c_id]._ObjStat.y);
	wstring userlevel = to_wstring(clients[c_id]._ObjStat.Level);
	wstring exp = to_wstring(clients[c_id]._ObjStat.Exp);
	wstring hp = to_wstring(clients[c_id]._ObjStat.HP);
	wstring hpmax = to_wstring(clients[c_id]._ObjStat.MaxHP);

	wstring storedProcedure = L"EXEC update_userinfo ";
	storedProcedure += mydb_id;
	storedProcedure += L", ";
	storedProcedure += race;
	storedProcedure += L", ";
	storedProcedure += xpos;
	storedProcedure += L", ";
	storedProcedure += ypos;
	storedProcedure += L", ";
	storedProcedure += userlevel;
	storedProcedure += L", ";
	storedProcedure += exp;
	storedProcedure += L", ";
	storedProcedure += hp;
	storedProcedure += L", ";
	storedProcedure += hpmax;

	retcode = SQLExecDirect(hstmt, (SQLWCHAR*)storedProcedure.c_str(), SQL_NTS);

	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
		retcode = SQLBindCol(hstmt, 1, SQL_C_LONG, &user_id, 10, &cbuser_id);
		retcode = SQLBindCol(hstmt, 2, SQL_C_LONG, &user_race, 10, &cbrace);
		retcode = SQLBindCol(hstmt, 3, SQL_C_LONG, &user_xpos, 10, &cbuser_xpos);
		retcode = SQLBindCol(hstmt, 4, SQL_C_LONG, &user_ypos, 10, &cbuser_ypos);
		retcode = SQLBindCol(hstmt, 5, SQL_C_LONG, &user_level, 10, &cbuser_level);
		retcode = SQLBindCol(hstmt, 6, SQL_C_LONG, &user_exp, 10, &cbuser_exp);
		retcode = SQLBindCol(hstmt, 7, SQL_C_LONG, &user_hp, 10, &cbuser_hp);
		retcode = SQLBindCol(hstmt, 8, SQL_C_LONG, &user_hpmax, 10, &cbuser_hpmax);

		for (int i = 0; ; i++) {
			retcode = SQLFetch(hstmt);
			break;
		}
	}
}

void SessionManager::ShowError(SQLHANDLE hHandle, SQLSMALLINT hType, RETCODE RetCode)
{
	SQLSMALLINT iRec = 0;
	SQLINTEGER iError;
	WCHAR wszMessage[1000];
	WCHAR wszState[SQL_SQLSTATE_SIZE + 1];
	if (RetCode == SQL_INVALID_HANDLE) {
		fwprintf(stderr, L"Invalid handle!\n");
		return;
	}
	while (SQLGetDiagRec(hType, hHandle, ++iRec, wszState, &iError, wszMessage,
		(SQLSMALLINT)(sizeof(wszMessage) / sizeof(WCHAR)), (SQLSMALLINT*)NULL) == SQL_SUCCESS) {
		// Hide data truncated..
		if (wcsncmp(wszState, L"01004", 5)) {
			fwprintf(stderr, L"[%5.5s] %s (%d)\n", wszState, wszMessage, iError);
		}
	}
}

void SessionManager::MoveNPC(int _npc_id, int _c_id)
{
	unordered_set<int> old_vl;

	for (auto& pl : ConnectedPlayer) {
	//	//if (clients[pl]._ObjStat.Sector != clients[_npc_id]._ObjStat.Sector) continue;
		if (clients[pl]._SessionState != ST_INGAME) continue;
		if (Distance(_npc_id, pl) <= RANGE) old_vl.insert(pl);
	}

	if (clients[_npc_id]._AttackType == ATTACKTYPE_PEACE)
		PathFinderPeace(_npc_id, _c_id);
	if (clients[_npc_id]._AttackType == ATTACKTYPE_AGRO)
		PathFinderAgro(_npc_id, _c_id);

	unordered_set<int> new_vl;

	for (auto& pl : ConnectedPlayer) {
		//if (clients[pl]._ObjStat.Sector != clients[_npc_id]._ObjStat.Sector) continue;
		if (clients[pl]._SessionState != ST_INGAME) continue;
		if (Distance(_npc_id, pl) <= RANGE) new_vl.insert(pl);
	}

	for (auto p_id : new_vl) {
		if (GSessionManager.clients[p_id]._ObjStat.Race != RACE::RACE_PLAYER) continue;
		clients[p_id]._ViewListLock.lock();
		if (clients[p_id]._ViewList.empty()) {
			clients[p_id]._ViewListLock.unlock();
			continue;
		}
		if (0 == clients[p_id]._ViewList.count(_npc_id)) {
			clients[p_id]._ViewList.insert(_npc_id);
			clients[p_id]._ViewListLock.unlock();
			clients[p_id].SendAddObjectPacket(_npc_id);
		}
		else {
			clients[p_id]._ViewListLock.unlock();
			clients[p_id].SendMovePacket(_npc_id, 0);
		}
	}
	for (auto p_id : old_vl) {
		if (GSessionManager.clients[p_id]._ObjStat.Race != RACE::RACE_PLAYER) continue;
		if (0 == new_vl.count(p_id)) {
			clients[p_id]._ViewListLock.lock();
			if (clients[p_id]._ViewList.count(_npc_id) == 1) {
				clients[p_id]._ViewList.erase(_npc_id);
				clients[p_id]._ViewListLock.unlock();
				clients[p_id].SendRemovePacket(_npc_id);
			}
			else
				clients[p_id]._ViewListLock.unlock();
		}
	}
}

void SessionManager::PathFinderPeace(int _npc_id, int _c_id)
{
	short x = clients[_npc_id]._ObjStat.x;
	short y = clients[_npc_id]._ObjStat.y;

	// 길찾기 알고리즘 적용해야함
	switch (rand() % 4) {
	case 0: if (IsPeaceMonsterMovePossible(_c_id, _npc_id, DIRECTION_UP) && y > 0) y--; break;
	case 1: if (IsPeaceMonsterMovePossible(_c_id, _npc_id, DIRECTION_DOWN) && y < W_HEIGHT - 1) y++; break;
	case 2: if (IsPeaceMonsterMovePossible(_c_id, _npc_id, DIRECTION_LEFT) && x > 0) x--; break;
	case 3: if (IsPeaceMonsterMovePossible(_c_id, _npc_id, DIRECTION_RIGHT) && x < W_WIDTH - 1) x++; break;
	}

	clients[_npc_id]._ObjStat.x = x;
	clients[_npc_id]._ObjStat.y = y;
}

void SessionManager::PathFinderAgro(int _npc_id, int _c_id)
{
	short x = clients[_npc_id]._ObjStat.x;
	short y = clients[_npc_id]._ObjStat.y;

	if (clients[_npc_id]._ObjStat.targetID == -1) 
		clients[_npc_id]._ObjStat.targetID = _c_id;	
	
	short target_x = clients[clients[_npc_id]._ObjStat.targetID]._ObjStat.x;
	short target_y = clients[clients[_npc_id]._ObjStat.targetID]._ObjStat.y;

	int direction = 4;

	if (Distance(_npc_id, _c_id) < 2) {
		// 몬스터가 플레이어를 공격한다
		HitPlayer(_npc_id, _c_id);
		return;
	}

	if (abs(x - target_x) > abs(y - target_y)) {
		if (x > target_x)
			direction = 2;
		else
			direction = 3;
	}
	else {
		if (y > target_y)
			direction = 0;
		else
			direction = 1;
	}

	// 길찾기 알고리즘 적용해야함
	switch (direction) {
	case 0:
		if (!IsPeaceMonsterMovePossible(_c_id, _npc_id, DIRECTION_UP)) {
			if (x > target_x) x--;
			else x++;
		}
		else if (y > 0) y--;
		break;
	case 1:
		if (!IsPeaceMonsterMovePossible(_c_id, _npc_id, DIRECTION_DOWN)) {
			if (x > target_x) x--;
			else x++;
		}
		else if (y < W_HEIGHT - 1) y++;
		break;
	case 2:
		if (!IsPeaceMonsterMovePossible(_c_id, _npc_id, DIRECTION_LEFT)) {
			if (y > target_y) y--;
			else y++;
		}
		else if (x > 0) x--;
		break;
	case 3:
		if (!IsPeaceMonsterMovePossible(_c_id, _npc_id, DIRECTION_RIGHT)) {
			if (y > target_y) y--;
			else y++;
		}
		else if (x < W_WIDTH - 1) x++;
		break;
	}

	clients[_npc_id]._ObjStat.x = x;
	clients[_npc_id]._ObjStat.y = y;
}

void SessionManager::HitNPC(int _p_id, int n_id)
{
	clients[n_id]._ObjStat.HP -= clients[_p_id]._ObjStat.Level * 50;

	char temp[BUF_SIZE];
	strcpy_s(temp, NoticeAttack(_p_id, n_id).c_str());
	clients[_p_id].SendNoticePacket(temp);
	clients[_p_id].SendStatChangePacket(_p_id, n_id);	// 몬스터 공격하여 체력이 변하면 플레이어에게 전송
	for (auto& pl : clients[_p_id]._ViewList) {
		if (clients[pl]._ObjStat.Race != RACE_PLAYER) continue;
		clients[pl].SendStatChangePacket(pl, n_id);	// 몬스터 공격하여 스탯이 변하면 근처 플레이어에게 전송
	}

	if (clients[n_id]._ObjStat.HP <= 0) {		// 몬스터 사망
		SaveUserInfo(clients[_p_id]._ObjStat.DBID, _p_id);

		clients[n_id]._SessionState = ST_SLEEP;
		clients[n_id]._ObjStat.IsDead = true;
		clients[_p_id].SendRemovePacket(n_id);	// 공격을 한 플레이어에게 전송

		CombatReward(_p_id, n_id);
		clients[_p_id].SendStatChangePacket(_p_id, _p_id);	// 몬스터 처치하여 스탯이 변하면 플레이어에게 전송

		string notice;
		notice += clients[_p_id]._ObjStat.Name;
		notice += " kill ";
		notice += clients[n_id]._ObjStat.Name;
		notice += " EXP ( ";
		notice += to_string(clients[_p_id]._ObjStat.Exp);
		notice += " / ";
		notice += to_string(clients[_p_id]._ObjStat.MaxExp);
		notice += " )";
		char temp[BUF_SIZE];
		strcpy_s(temp, notice.c_str());
		clients[_p_id].SendNoticePacket(temp);

		for (auto& party : clients[_p_id]._MyParty) {
			CombatReward(party, n_id);
			clients[party].SendStatChangePacket(party, party);

			notice.clear();
			notice += "Party Bonus";
			notice += " EXP ( ";
			notice += to_string(clients[party]._ObjStat.Exp);
			notice += " / ";
			notice += to_string(clients[party]._ObjStat.MaxExp);
			notice += " )";
			char temp[BUF_SIZE];
			strcpy_s(temp, notice.c_str());
			clients[party].SendNoticePacket(temp);
		}

		// 시야 안의 모든 플레이어들에게 REMOVE패킷 전송
		for (auto& pl : clients[_p_id]._ViewList) {
			if (clients[pl]._ObjStat.Race != RACE_PLAYER) continue;
			clients[pl].SendRemovePacket(n_id);			// 근처 플레이어들에게도 전송
			clients[pl].SendStatChangePacket(pl, _p_id);	// 몬스터 처치하여 스탯이 변하면 근처 플레이어에게 전송	
		}
	}
}

void SessionManager::HitPlayer(int _n_id, int _p_id)
{
	if (clients[_n_id]._ObjStat.IsDead) return;

	int AttackPower = clients[_n_id]._ObjStat.Level * 2;
	clients[_p_id]._ObjStat.HP -= AttackPower;

	char temp[BUF_SIZE];
	strcpy_s(temp, NoticeAttack(_n_id, _p_id).c_str());
	clients[_p_id].SendNoticePacket(temp);
	clients[_p_id].SendStatChangePacket(_p_id, _p_id);
	clients[_p_id]._ViewListLock.lock();
	for (auto& pl : clients[_p_id]._ViewList) {
		if (clients[pl]._ObjStat.Race != RACE_PLAYER) continue;
		clients[pl].SendStatChangePacket(pl, _p_id);
	}
	clients[_p_id]._ViewListLock.unlock();

	// 플레이어 사망
	if (clients[_p_id]._ObjStat.HP <= 0) {
		clients[_p_id]._ObjStat.HP = clients[_p_id]._ObjStat.MaxHP;
		clients[_p_id]._ObjStat.Exp = clients[_p_id]._ObjStat.Exp / 2;


		clients[_p_id]._ObjStat.x = 0;
		clients[_p_id]._ObjStat.y = 0;
		/*clients[_p_id]._ObjStat.x = rand() % W_WIDTH;
		clients[_p_id]._ObjStat.y = rand() % W_HEIGHT;*/
		
		clients[_p_id]._ViewList.clear();

		for (int obj = 0; obj < MAX_USER + NUM_NPC; ++obj) {
			if (clients[obj]._SessionState == ST_FREE) continue;
			if (clients[obj]._ObjStat.IsDead == true) continue;
			if (obj == _p_id) continue;
			//if (clients[_p_id]._ObjStat.Sector != clients[obj]._ObjStat.Sector) continue;
			if (RANGE > Distance(_p_id, obj)) {
				clients[_p_id].SendAddObjectPacket(obj);
				clients[_p_id]._ViewListLock.lock();
				clients[_p_id]._ViewList.insert(obj);
				clients[_p_id]._ViewListLock.unlock();
				clients[obj]._SessionState = ST_INGAME;
			}
		}

		string notice;
		notice += clients[_p_id]._ObjStat.Name;
		notice += " Dead!";
		strcpy_s(temp, notice.c_str());
		clients[_p_id].SendNoticePacket(temp);
		clients[_p_id].SendStatChangePacket(_p_id, _p_id);
		clients[_p_id].SendMovePacket(_p_id, 0);

		for (auto& pl : clients[_p_id]._ViewList) {
			if (clients[pl]._ObjStat.Race != RACE_PLAYER) continue;
			clients[_p_id].SendNoticePacket(temp);
			clients[pl].SendStatChangePacket(pl, _p_id);
			clients[pl].SendMovePacket(_p_id, 0);
		}
	}
}

void SessionManager::CombatReward(int p_id, int n_id)
{
	// 몬스터 처치 보상
	int rewardEXP = clients[n_id]._ObjStat.Level * clients[n_id]._ObjStat.Level * 2;
	if (clients[n_id]._AttackType == ATTACKTYPE_AGRO) {
		rewardEXP = rewardEXP * 2;
		if (clients[n_id]._MoveType == MOVETYPE_ROAMING)
			rewardEXP = rewardEXP * 2;
	}

	clients[p_id]._ObjStat.Exp += rewardEXP;

	if (clients[p_id]._ObjStat.Exp > clients[p_id]._ObjStat.MaxExp) {
		clients[p_id]._ObjStat.Level += 1;
		clients[p_id]._ObjStat.MaxHP = clients[p_id]._ObjStat.Level * 100;
		clients[p_id]._ObjStat.HP = clients[p_id]._ObjStat.MaxHP;
		clients[p_id]._ObjStat.MaxExp = clients[p_id]._ObjStat.Level * 100;
		clients[p_id]._ObjStat.Exp = 0;

		string notice;
		notice += clients[p_id]._ObjStat.Name;
		notice += " Level UP! to ";
		notice += to_string(clients[p_id]._ObjStat.Level);
		char temp[BUF_SIZE];
		strcpy_s(temp, notice.c_str());
		clients[p_id].SendNoticePacket(temp);
	}
}

int SessionManager::DistanceBlock(int a, int b)
{
	return abs(clients[a]._ObjStat.x - blocks[b].x)
		+ abs(clients[a]._ObjStat.y - blocks[b].y);
}

int SessionManager::Distance(int a, int b)
{
	return abs(clients[a]._ObjStat.x - clients[b]._ObjStat.x)
		+ abs(clients[a]._ObjStat.y - clients[b]._ObjStat.y);
}

bool SessionManager::IsMovePossible(int _id, DIRECTION _direction)
{
	unordered_set<int> block_vl = clients[_id]._ViewListBlock;
	unordered_set<int> obj_vl = clients[_id]._ViewList;


	for (auto& obj : clients[_id]._ViewList) {
		if (clients[obj]._SessionState == ST_SLEEP)
			obj_vl.erase(obj);
	}

	switch (_direction)
	{
	case DIRECTION_UP:
		for (auto& obj : obj_vl) {
			if (clients[_id]._ObjStat.x == clients[obj]._ObjStat.x &&
				clients[_id]._ObjStat.y == clients[obj]._ObjStat.y + 1)
				return false;
		}
		break;
	case DIRECTION_DOWN:
		for (auto& obj : obj_vl) {
			if (clients[_id]._ObjStat.x == clients[obj]._ObjStat.x &&
				clients[_id]._ObjStat.y == clients[obj]._ObjStat.y - 1)
				return false;
		}
		break;
	case DIRECTION_LEFT:
		for (auto& obj : obj_vl) {
			if (clients[_id]._ObjStat.x == clients[obj]._ObjStat.x + 1 &&
				clients[_id]._ObjStat.y == clients[obj]._ObjStat.y)
				return false;
		}
		break;
	case DIRECTION_RIGHT:
		for (auto& obj : obj_vl) {
			if (clients[_id]._ObjStat.x == clients[obj]._ObjStat.x - 1 &&
				clients[_id]._ObjStat.y == clients[obj]._ObjStat.y)
				return false;
		}
		break;
	}

	switch (_direction)
	{
	case DIRECTION_UP:
		for (auto& bl : block_vl) {
			if (((clients[_id]._ObjStat.x == blocks[bl].x + 2) || (clients[_id]._ObjStat.x == blocks[bl].x + 1)) &&
				clients[_id]._ObjStat.y == (blocks[bl].y + 1))
				return false;
		}
		break;
	case DIRECTION_DOWN:
		for (auto& bl : block_vl) {
			if (((clients[_id]._ObjStat.x == blocks[bl].x + 2) || (clients[_id]._ObjStat.x == blocks[bl].x + 1)) &&
				clients[_id]._ObjStat.y == (blocks[bl].y - 1))
				return false;
		}
		break;
	case DIRECTION_LEFT:
		for (auto& bl : block_vl) {
			if ((clients[_id]._ObjStat.x == blocks[bl].x + 3) &&
				(clients[_id]._ObjStat.y == blocks[bl].y))
				return false;
		}
		break;
	case DIRECTION_RIGHT:
		for (auto& bl : block_vl) {
			if ((clients[_id]._ObjStat.x == blocks[bl].x) &&
				(clients[_id]._ObjStat.y == blocks[bl].y))
				return false;
		}
		break;
	}

	return true;
}

bool SessionManager::IsPeaceMonsterMovePossible(int _cid, int _mid, DIRECTION _direction)
{
	clients[_cid]._ViewListLock.lock();
	unordered_set<int> block_vl = clients[_cid]._ViewListBlock;
	unordered_set<int> obj_vl = clients[_cid]._ViewList;
	clients[_cid]._ViewListLock.unlock();

	switch (_direction)
	{
	case DIRECTION_UP:
		for (auto& obj : obj_vl) {
			if (clients[_mid]._ObjStat.x == clients[obj]._ObjStat.x &&
				clients[_mid]._ObjStat.y == clients[obj]._ObjStat.y + 1)
				return false;
		}
		break;
	case DIRECTION_DOWN:
		for (auto& obj : obj_vl) {
			if (clients[_mid]._ObjStat.x == clients[obj]._ObjStat.x &&
				clients[_mid]._ObjStat.y == clients[obj]._ObjStat.y - 1)
				return false;
		}
		break;
	case DIRECTION_LEFT:
		for (auto& obj : obj_vl) {
			if (clients[_mid]._ObjStat.x == clients[obj]._ObjStat.x + 1 &&
				clients[_mid]._ObjStat.y == clients[obj]._ObjStat.y)
				return false;
		}
		break;
	case DIRECTION_RIGHT:
		for (auto& obj : obj_vl) {
			if (clients[_mid]._ObjStat.x == clients[obj]._ObjStat.x - 1 &&
				clients[_mid]._ObjStat.y == clients[obj]._ObjStat.y)
				return false;
		}
		break;
	}

	switch (_direction)
	{
	case DIRECTION_UP:
		for (auto& bl : block_vl) {
			if (((clients[_mid]._ObjStat.x == blocks[bl].x + 2) || (clients[_mid]._ObjStat.x == blocks[bl].x + 1)) &&
				clients[_mid]._ObjStat.y == (blocks[bl].y + 1))
				return false;
		}
		break;
	case DIRECTION_DOWN:
		for (auto& bl : block_vl) {
			if (((clients[_mid]._ObjStat.x == blocks[bl].x + 2) || (clients[_mid]._ObjStat.x == blocks[bl].x + 1)) &&
				clients[_mid]._ObjStat.y == (blocks[bl].y - 1))
				return false;
		}
		break;
	case DIRECTION_LEFT:
		for (auto& bl : block_vl) {
			if ((clients[_mid]._ObjStat.x == blocks[bl].x + 3) &&
				(clients[_mid]._ObjStat.y == blocks[bl].y))
				return false;
		}
		break;
	case DIRECTION_RIGHT:
		for (auto& bl : block_vl) {
			if ((clients[_mid]._ObjStat.x == blocks[bl].x) &&
				(clients[_mid]._ObjStat.y == blocks[bl].y))
				return false;
		}
		break;
	}


	return true;
}

string SessionManager::NoticeAttack(int _attackID, int _targetID)
{
	string notice;
	notice += clients[_attackID]._ObjStat.Name;
	notice += " Attacks a ";
	notice += clients[_targetID]._ObjStat.Name;
	if (_targetID < MAX_USER)
		notice += " / Player HP to ";
	else
		notice += " / Monster HP to ";
	notice += to_string(clients[_targetID]._ObjStat.HP);

	return notice;
}

