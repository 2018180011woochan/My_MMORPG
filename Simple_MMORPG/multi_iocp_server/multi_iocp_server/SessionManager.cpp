#include "pch.h"
#include "SessionManager.h"

SessionManager GSessionManager;

int SessionManager::get_new_client_id()
{
	for (int i = 0; i < MAX_USER; ++i) {
		clients[i]._lock.lock();
		if (clients[i]._s_state == ST_FREE) {
			clients[i]._s_state = ST_ACCEPTED;
			clients[i]._lock.unlock();
			return i;
		}
		clients[i]._lock.unlock();
	}
	return -1;
}

void SessionManager::Init_npc()
{
	for (int i = MAX_USER; i < NUM_NPC + MAX_USER; ++i)
		clients[i]._obj_stat._id = i;
	cout << "NPC initialize Begin.\n";

	for (int i = MAX_USER; i < MAX_USER + 50000; ++i)
	{
		// Skeleton
		clients[i]._s_state = ST_SLEEP;
		clients[i]._obj_stat.race = RACE::RACE_SKELETON;
		clients[i]._obj_stat.level = 1;
		clients[i]._obj_stat.hpmax = clients[i]._obj_stat.level * 100;
		clients[i]._obj_stat.hp = clients[i]._obj_stat.hpmax;
		clients[i]._movetype = MOVETYPE::MOVETYPE_FIX;
		clients[i]._attacktype = ATTACKTYPE::ATTACKTYPE_PEACE;
		strcpy_s(clients[i]._obj_stat._name, "Skeleton");

		SetSector(RACE_SKELETON, i);
	}
	for (int i = MAX_USER + 50000; i < MAX_USER + 100000; ++i)
	{
		// Wraith
		clients[i]._s_state = ST_SLEEP;
		clients[i]._obj_stat.race = RACE::RACE_WRIATH;
		clients[i]._obj_stat.level = 2;
		clients[i]._obj_stat.hpmax = clients[i]._obj_stat.level * 100;
		clients[i]._obj_stat.hp = clients[i]._obj_stat.hpmax;
		clients[i]._movetype = MOVETYPE::MOVETYPE_FIX;
		clients[i]._attacktype = ATTACKTYPE::ATTACKTYPE_AGRO;
		strcpy_s(clients[i]._obj_stat._name, "Wriath");

		SetSector(RACE_WRIATH, i);
	}
	for (int i = MAX_USER + 100000; i < MAX_USER + NUM_NPC; ++i)
	{
		// Devil
		clients[i]._s_state = ST_SLEEP;
		clients[i]._obj_stat.race = RACE::RACE_DEVIL;
		clients[i]._obj_stat.level = 3;
		clients[i]._obj_stat.hpmax = clients[i]._obj_stat.level * 100;
		clients[i]._obj_stat.hp = clients[i]._obj_stat.hpmax;
		clients[i]._movetype = MOVETYPE::MOVETYPE_ROAMING;
		clients[i]._attacktype = ATTACKTYPE::ATTACKTYPE_PEACE;
		strcpy_s(clients[i]._obj_stat._name, "Devil");

		SetSector(RACE_DEVIL, i);
	}

	cout << "NPC Initialization complete.\n";
}

void SessionManager::Init_Block()
{
	for (int i = 0; i < NUM_BLOCK; ++i)
	{
		blocks[i].blockID = i;
		blocks[i].x = rand() % W_WIDTH;
		blocks[i].y = rand() % W_WIDTH;
		SetSector(RACE_BLOCK, i);
	}
}

void SessionManager::initialize_DB()
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

bool SessionManager::isAllowAccess(int db_id, int cid)
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
				clients[cid]._obj_stat.race = user_race;
				clients[cid]._obj_stat.x = user_xpos;
				clients[cid]._obj_stat.y = user_ypos;
				clients[cid]._obj_stat.level = user_level;
				clients[cid]._obj_stat.exp = user_exp;
				clients[cid]._obj_stat.hp = user_hp;
				clients[cid]._obj_stat.hpmax = user_hpmax;
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

void SessionManager::Save_UserInfo(int db_id, int c_id)
{
	retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);

	wstring mydb_id = to_wstring(clients[c_id]._obj_stat._db_id);
	wstring race = to_wstring(clients[c_id]._obj_stat.race);
	wstring xpos = to_wstring(clients[c_id]._obj_stat.x);
	wstring ypos = to_wstring(clients[c_id]._obj_stat.y);
	wstring userlevel = to_wstring(clients[c_id]._obj_stat.level);
	wstring exp = to_wstring(clients[c_id]._obj_stat.exp);
	wstring hp = to_wstring(clients[c_id]._obj_stat.hp);
	wstring hpmax = to_wstring(clients[c_id]._obj_stat.hpmax);

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

SECTOR SessionManager::GetSector(int _race, int _id)
{
	if (_race == RACE_BLOCK)
		return blocks[_id].sector;
	else
		return clients[_id]._obj_stat.sector;

	return SECTOR_END;
}

void SessionManager::SetSector(int _race, int _id)
{
	if (_race == RACE_BLOCK) {
		if (blocks[_id].x < Half_width) {	// 1,3 섹터
			if (blocks[_id].y < Half_height)
				blocks[_id].sector = SECTOR_1;
			else
				blocks[_id].sector = SECTOR_3;
		}
		else {								// 2,4 섹터
			if (blocks[_id].y < Half_height)
				blocks[_id].sector = SECTOR_2;
			else
				blocks[_id].sector = SECTOR_4;
		}
	}
	else {
		if (clients[_id]._obj_stat.x < Half_width) {	// 1,3 섹터
			if (clients[_id]._obj_stat.y < Half_height)
				clients[_id]._obj_stat.sector = SECTOR_1;
			else
				clients[_id]._obj_stat.sector = SECTOR_3;
		}
		else {								// 2,4 섹터
			if (clients[_id]._obj_stat.y < Half_height)
				clients[_id]._obj_stat.sector = SECTOR_2;
			else
				clients[_id]._obj_stat.sector = SECTOR_4;
		}
	}
}

void SessionManager::Move_NPC(int _npc_id, int _c_id)
{
	unordered_set<int> old_vl;

	for (auto& pl : ConnectedPlayer) {
		if (clients[pl]._obj_stat.sector != clients[_npc_id]._obj_stat.sector) continue;
		if (clients[pl]._s_state != ST_INGAME) continue;
		if (distance(_npc_id, pl) <= RANGE) old_vl.insert(pl);
	}

	if (clients[_npc_id]._attacktype == ATTACKTYPE_PEACE)
		PathFinder_Peace(_npc_id, _c_id);
	if (clients[_npc_id]._attacktype == ATTACKTYPE_AGRO)
		PathFinder_Agro(_npc_id, _c_id);

	unordered_set<int> new_vl;

	for (auto& pl : ConnectedPlayer) {
		if (clients[pl]._obj_stat.sector != clients[_npc_id]._obj_stat.sector) continue;
		if (clients[pl]._s_state != ST_INGAME) continue;
		if (distance(_npc_id, pl) <= RANGE) new_vl.insert(pl);
	}

	for (auto p_id : new_vl) {
		if (GSessionManager.clients[p_id]._obj_stat.race != RACE::RACE_PLAYER) continue;
		clients[p_id]._ViewListLock.lock();
		if (0 == clients[p_id].view_list.count(_npc_id)) {
			clients[p_id].view_list.insert(_npc_id);
			clients[p_id]._ViewListLock.unlock();
			clients[p_id].send_add_object(_npc_id);
		}
		else {
			clients[p_id]._ViewListLock.unlock();
			clients[p_id].send_move_packet(_npc_id, 0);
		}
	}
	for (auto p_id : old_vl) {
		if (GSessionManager.clients[p_id]._obj_stat.race != RACE::RACE_PLAYER) continue;
		if (0 == new_vl.count(p_id)) {
			clients[p_id]._ViewListLock.lock();
			if (clients[p_id].view_list.count(_npc_id) == 1) {
				clients[p_id].view_list.erase(_npc_id);
				clients[p_id]._ViewListLock.unlock();
				clients[p_id].Send_Remove_Packet(_npc_id);
			}
			else
				clients[p_id]._ViewListLock.unlock();
		}
	}
}

void SessionManager::PathFinder_Peace(int _npc_id, int _c_id)
{
	short x = clients[_npc_id]._obj_stat.x;
	short y = clients[_npc_id]._obj_stat.y;

	// 길찾기 알고리즘 적용해야함
	switch (rand() % 4) {
	case 0: if (isPeaceMonsterMovePossible(_c_id, _npc_id, DIRECTION_UP) && y > 0) y--; break;
	case 1: if (isPeaceMonsterMovePossible(_c_id, _npc_id, DIRECTION_DOWN) && y < W_HEIGHT - 1) y++; break;
	case 2: if (isPeaceMonsterMovePossible(_c_id, _npc_id, DIRECTION_LEFT) && x > 0) x--; break;
	case 3: if (isPeaceMonsterMovePossible(_c_id, _npc_id, DIRECTION_RIGHT) && x < W_WIDTH - 1) x++; break;
	}

	clients[_npc_id]._obj_stat.x = x;
	clients[_npc_id]._obj_stat.y = y;
}

void SessionManager::PathFinder_Agro(int _npc_id, int _c_id)
{
	short x = clients[_npc_id]._obj_stat.x;
	short y = clients[_npc_id]._obj_stat.y;

	short target_x = clients[_c_id]._obj_stat.x;
	short target_y = clients[_c_id]._obj_stat.y;

	int direction = 4;

	if (distance(_npc_id, _c_id) < 2) {
		// 몬스터가 플레이어를 공격한다
		Hit_Player(_npc_id, _c_id);
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
		if (!isPeaceMonsterMovePossible(_c_id, _npc_id, DIRECTION_UP)) {
			if (x > target_x) x--;
			else x++;
		}
		else if (y > 0) y--;
		break;
	case 1:
		if (!isPeaceMonsterMovePossible(_c_id, _npc_id, DIRECTION_DOWN)) {
			if (x > target_x) x--;
			else x++;
		}
		else if (y < W_HEIGHT - 1) y++;
		break;
	case 2:
		if (!isPeaceMonsterMovePossible(_c_id, _npc_id, DIRECTION_LEFT)) {
			if (y > target_y) y--;
			else y++;
		}
		else if (x > 0) x--;
		break;
	case 3:
		if (!isPeaceMonsterMovePossible(_c_id, _npc_id, DIRECTION_RIGHT)) {
			if (y > target_y) y--;
			else y++;
		}
		else if (x < W_WIDTH - 1) x++;
		break;
	}

	clients[_npc_id]._obj_stat.x = x;
	clients[_npc_id]._obj_stat.y = y;
}

void SessionManager::Hit_NPC(int _p_id, int n_id)
{
	clients[n_id]._obj_stat.hp -= clients[_p_id]._obj_stat.level * 50;

	char temp[BUF_SIZE];
	strcpy_s(temp, Notice_Attack(_p_id, n_id).c_str());
	clients[_p_id].Send_Notice_Packet(temp);
	clients[_p_id].Send_StatChange_Packet(_p_id, n_id);	// 몬스터 공격하여 체력이 변하면 플레이어에게 전송
	for (auto& pl : clients[_p_id].view_list) {
		if (clients[pl]._obj_stat.race != RACE_PLAYER) continue;
		clients[pl].Send_StatChange_Packet(pl, n_id);	// 몬스터 공격하여 스탯이 변하면 근처 플레이어에게 전송
	}

	if (clients[n_id]._obj_stat.hp <= 0) {		// 몬스터 사망
		Save_UserInfo(clients[_p_id]._obj_stat._db_id, _p_id);

		clients[n_id]._s_state = ST_SLEEP;
		clients[n_id]._obj_stat.isDead = true;
		clients[_p_id].Send_Remove_Packet(n_id);	// 공격을 한 플레이어에게 전송

		Combat_Reward(_p_id, n_id);
		clients[_p_id].Send_StatChange_Packet(_p_id, _p_id);	// 몬스터 처치하여 스탯이 변하면 플레이어에게 전송

		string notice;
		notice += clients[_p_id]._obj_stat._name;
		notice += " kill ";
		notice += clients[n_id]._obj_stat._name;
		notice += " EXP ( ";
		notice += to_string(clients[_p_id]._obj_stat.exp);
		notice += " / ";
		notice += to_string(clients[_p_id]._obj_stat.maxexp);
		notice += " )";
		char temp[BUF_SIZE];
		strcpy_s(temp, notice.c_str());
		clients[_p_id].Send_Notice_Packet(temp);

		for (auto& party : clients[_p_id].my_party) {
			Combat_Reward(party, n_id);
			clients[party].Send_StatChange_Packet(party, party);

			notice.clear();
			notice += "Party Bonus";
			notice += " EXP ( ";
			notice += to_string(clients[party]._obj_stat.exp);
			notice += " / ";
			notice += to_string(clients[party]._obj_stat.maxexp);
			notice += " )";
			char temp[BUF_SIZE];
			strcpy_s(temp, notice.c_str());
			clients[party].Send_Notice_Packet(temp);
		}

		// 시야 안의 모든 플레이어들에게 REMOVE패킷 전송
		for (auto& pl : clients[_p_id].view_list) {
			if (clients[pl]._obj_stat.race != RACE_PLAYER) continue;
			clients[pl].Send_Remove_Packet(n_id);			// 근처 플레이어들에게도 전송
			clients[pl].Send_StatChange_Packet(pl, _p_id);	// 몬스터 처치하여 스탯이 변하면 근처 플레이어에게 전송	
		}
	}
}

void SessionManager::Hit_Player(int _n_id, int _p_id)
{
	if (clients[_n_id]._obj_stat.isDead) return;

	int AttackPower = clients[_n_id]._obj_stat.level * 2;
	clients[_p_id]._obj_stat.hp -= AttackPower;

	char temp[BUF_SIZE];
	strcpy_s(temp, Notice_Attack(_n_id, _p_id).c_str());
	clients[_p_id].Send_Notice_Packet(temp);
	clients[_p_id].Send_StatChange_Packet(_p_id, _p_id);
	for (auto& pl : clients[_p_id].view_list) {
		if (clients[pl]._obj_stat.race != RACE_PLAYER) continue;
		clients[pl].Send_StatChange_Packet(pl, _p_id);
	}

	// 플레이어 사망
	if (clients[_p_id]._obj_stat.hp <= 0) {
		clients[_p_id]._obj_stat.hp = clients[_p_id]._obj_stat.hpmax;
		clients[_p_id]._obj_stat.exp = clients[_p_id]._obj_stat.exp / 2;
		//if (!isStressTest) {
		//clients[_p_id]._obj_stat.x = 0;
		//clients[_p_id]._obj_stat.y = 0;
		//}

		clients[_p_id]._obj_stat.x = rand() % W_WIDTH;
		clients[_p_id]._obj_stat.y = rand() % W_HEIGHT;
		clients[_p_id]._obj_stat.x = 12;
		clients[_p_id]._obj_stat.y = 15;
		SetSector(RACE_PLAYER, _p_id);
		
		clients[_p_id].view_list.clear();

		for (int obj = 0; obj < MAX_USER + NUM_NPC; ++obj) {
			if (clients[obj]._s_state == ST_FREE) continue;
			if (clients[obj]._obj_stat.isDead == true) continue;
			if (obj == _p_id) continue;
			if (clients[_p_id]._obj_stat.sector != clients[obj]._obj_stat.sector) continue;
			if (RANGE > distance(_p_id, obj)) {
				clients[_p_id].send_add_object(obj);
				clients[_p_id]._ViewListLock.lock();
				clients[_p_id].view_list.insert(obj);
				clients[_p_id]._ViewListLock.unlock();
				clients[obj]._s_state = ST_INGAME;
			}
		}

		string notice;
		notice += clients[_p_id]._obj_stat._name;
		notice += " Dead!";
		strcpy_s(temp, notice.c_str());
		clients[_p_id].Send_Notice_Packet(temp);
		clients[_p_id].Send_StatChange_Packet(_p_id, _p_id);
		clients[_p_id].send_move_packet(_p_id, 0);

		for (auto& pl : clients[_p_id].view_list) {
			if (clients[pl]._obj_stat.race != RACE_PLAYER) continue;
			clients[_p_id].Send_Notice_Packet(temp);
			clients[pl].Send_StatChange_Packet(pl, _p_id);
			clients[pl].send_move_packet(_p_id, 0);
		}
	}
}

void SessionManager::Combat_Reward(int p_id, int n_id)
{
	// 몬스터 처치 보상
	int rewardEXP = clients[n_id]._obj_stat.level * clients[n_id]._obj_stat.level * 2;
	if (clients[n_id]._attacktype == ATTACKTYPE_AGRO) {
		rewardEXP = rewardEXP * 2;
		if (clients[n_id]._movetype == MOVETYPE_ROAMING)
			rewardEXP = rewardEXP * 2;
	}

	clients[p_id]._obj_stat.exp += rewardEXP;

	if (clients[p_id]._obj_stat.exp > clients[p_id]._obj_stat.maxexp) {
		clients[p_id]._obj_stat.level += 1;
		clients[p_id]._obj_stat.hpmax = clients[p_id]._obj_stat.level * 100;
		clients[p_id]._obj_stat.hp = clients[p_id]._obj_stat.hpmax;
		clients[p_id]._obj_stat.maxexp = clients[p_id]._obj_stat.level * 100;
		clients[p_id]._obj_stat.exp = 0;

		string notice;
		notice += clients[p_id]._obj_stat._name;
		notice += " Level UP! to ";
		notice += to_string(clients[p_id]._obj_stat.level);
		char temp[BUF_SIZE];
		strcpy_s(temp, notice.c_str());
		clients[p_id].Send_Notice_Packet(temp);
	}
}

int SessionManager::distance_block(int a, int b)
{
	return abs(clients[a]._obj_stat.x - blocks[b].x)
		+ abs(clients[a]._obj_stat.y - blocks[b].y);
}

int SessionManager::distance(int a, int b)
{
	return abs(clients[a]._obj_stat.x - clients[b]._obj_stat.x)
		+ abs(clients[a]._obj_stat.y - clients[b]._obj_stat.y);
}

bool SessionManager::isMovePossible(int _id, DIRECTION _direction)
{
	unordered_set<int> block_vl = clients[_id].block_view_list;
	unordered_set<int> obj_vl = clients[_id].view_list;


	for (auto& obj : clients[_id].view_list) {
		if (clients[obj]._s_state == ST_SLEEP)
			obj_vl.erase(obj);
	}

	switch (_direction)
	{
	case DIRECTION_UP:
		for (auto& obj : obj_vl) {
			if (clients[_id]._obj_stat.x == clients[obj]._obj_stat.x &&
				clients[_id]._obj_stat.y == clients[obj]._obj_stat.y + 1)
				return false;
		}
		break;
	case DIRECTION_DOWN:
		for (auto& obj : obj_vl) {
			if (clients[_id]._obj_stat.x == clients[obj]._obj_stat.x &&
				clients[_id]._obj_stat.y == clients[obj]._obj_stat.y - 1)
				return false;
		}
		break;
	case DIRECTION_LEFT:
		for (auto& obj : obj_vl) {
			if (clients[_id]._obj_stat.x == clients[obj]._obj_stat.x + 1 &&
				clients[_id]._obj_stat.y == clients[obj]._obj_stat.y)
				return false;
		}
		break;
	case DIRECTION_RIGHT:
		for (auto& obj : obj_vl) {
			if (clients[_id]._obj_stat.x == clients[obj]._obj_stat.x - 1 &&
				clients[_id]._obj_stat.y == clients[obj]._obj_stat.y)
				return false;
		}
		break;
	}

	switch (_direction)
	{
	case DIRECTION_UP:
		for (auto& bl : block_vl) {
			if (((clients[_id]._obj_stat.x == blocks[bl].x + 2) || (clients[_id]._obj_stat.x == blocks[bl].x + 1)) &&
				clients[_id]._obj_stat.y == (blocks[bl].y + 1))
				return false;
		}
		break;
	case DIRECTION_DOWN:
		for (auto& bl : block_vl) {
			if (((clients[_id]._obj_stat.x == blocks[bl].x + 2) || (clients[_id]._obj_stat.x == blocks[bl].x + 1)) &&
				clients[_id]._obj_stat.y == (blocks[bl].y - 1))
				return false;
		}
		break;
	case DIRECTION_LEFT:
		for (auto& bl : block_vl) {
			if ((clients[_id]._obj_stat.x == blocks[bl].x + 3) &&
				(clients[_id]._obj_stat.y == blocks[bl].y))
				return false;
		}
		break;
	case DIRECTION_RIGHT:
		for (auto& bl : block_vl) {
			if ((clients[_id]._obj_stat.x == blocks[bl].x) &&
				(clients[_id]._obj_stat.y == blocks[bl].y))
				return false;
		}
		break;
	}

	return true;
}

bool SessionManager::isPeaceMonsterMovePossible(int _cid, int _mid, DIRECTION _direction)
{
	unordered_set<int> block_vl = clients[_cid].block_view_list;

	unordered_set<int> obj_vl = clients[_cid].view_list;

	switch (_direction)
	{
	case DIRECTION_UP:
		for (auto& obj : obj_vl) {
			if (clients[_mid]._obj_stat.x == clients[obj]._obj_stat.x &&
				clients[_mid]._obj_stat.y == clients[obj]._obj_stat.y + 1)
				return false;
		}
		break;
	case DIRECTION_DOWN:
		for (auto& obj : obj_vl) {
			if (clients[_mid]._obj_stat.x == clients[obj]._obj_stat.x &&
				clients[_mid]._obj_stat.y == clients[obj]._obj_stat.y - 1)
				return false;
		}
		break;
	case DIRECTION_LEFT:
		for (auto& obj : obj_vl) {
			if (clients[_mid]._obj_stat.x == clients[obj]._obj_stat.x + 1 &&
				clients[_mid]._obj_stat.y == clients[obj]._obj_stat.y)
				return false;
		}
		break;
	case DIRECTION_RIGHT:
		for (auto& obj : obj_vl) {
			if (clients[_mid]._obj_stat.x == clients[obj]._obj_stat.x - 1 &&
				clients[_mid]._obj_stat.y == clients[obj]._obj_stat.y)
				return false;
		}
		break;
	}

	switch (_direction)
	{
	case DIRECTION_UP:
		for (auto& bl : block_vl) {
			if (((clients[_mid]._obj_stat.x == blocks[bl].x + 2) || (clients[_mid]._obj_stat.x == blocks[bl].x + 1)) &&
				clients[_mid]._obj_stat.y == (blocks[bl].y + 1))
				return false;
		}
		break;
	case DIRECTION_DOWN:
		for (auto& bl : block_vl) {
			if (((clients[_mid]._obj_stat.x == blocks[bl].x + 2) || (clients[_mid]._obj_stat.x == blocks[bl].x + 1)) &&
				clients[_mid]._obj_stat.y == (blocks[bl].y - 1))
				return false;
		}
		break;
	case DIRECTION_LEFT:
		for (auto& bl : block_vl) {
			if ((clients[_mid]._obj_stat.x == blocks[bl].x + 3) &&
				(clients[_mid]._obj_stat.y == blocks[bl].y))
				return false;
		}
		break;
	case DIRECTION_RIGHT:
		for (auto& bl : block_vl) {
			if ((clients[_mid]._obj_stat.x == blocks[bl].x) &&
				(clients[_mid]._obj_stat.y == blocks[bl].y))
				return false;
		}
		break;
	}


	return true;
}

string SessionManager::Notice_Attack(int _attackID, int _targetID)
{
	string notice;
	notice += clients[_attackID]._obj_stat._name;
	notice += " Attacks a ";
	notice += clients[_targetID]._obj_stat._name;
	if (_targetID < MAX_USER)
		notice += " / Player HP to ";
	else
		notice += " / Monster HP to ";
	notice += to_string(clients[_targetID]._obj_stat.hp);

	return notice;
}

