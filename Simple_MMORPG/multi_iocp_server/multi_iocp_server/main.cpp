#include <iostream>
#include <array>
#include <WS2tcpip.h>
#include <MSWSock.h>

#include <thread>
#include <vector>
#include <mutex>
#include <unordered_set>
#include <queue>
#include <chrono>
#include <string>
#include <atomic>

#include <windows.h>  
#include <locale.h>

#define UNICODE  
#include <sqlext.h>  

#include "protocol.h"
#include "Enum.h"

constexpr int RANGE = 15;
constexpr int MONSTER_RANGE = 5;

#pragma comment(lib, "WS2_32.lib")
#pragma comment(lib, "MSWSock.lib")
using namespace std;

// 내일할거 : 파티하고 자잘한버그고치고(체력바, 이름색깔)
// 밸런스패치쫌하고(경험치, )

// 코드도 쫌 나누고,,
//내일까지못하겠다 수요일까지 하는걸로,,,

//////////////// DB //////////////
SQLHENV henv;
SQLHDBC hdbc;
SQLHSTMT hstmt = 0;
SQLRETURN retcode;

SQLINTEGER user_id, user_race, user_xpos, user_ypos, user_level, user_exp, user_hp, user_hpmax;
SQLLEN cbuser_id = 0, cbrace = 0, cbuser_xpos = 0, cbuser_ypos = 0, cbuser_level,
cbuser_exp, cbuser_hp, cbuser_hpmax;
/// //////////////////////////////

void ShowError(SQLHANDLE hHandle, SQLSMALLINT hType, RETCODE RetCode);
void Init_npc();
void Init_Block();
void initialize_DB();
void Move_NPC(int _npc_id, int _c_id);
void PathFinder_Peace(int _npc_id, int _c_id);
void PathFinder_Agro(int _npc_id, int _c_id);
void Hit_NPC(int _p_id, int n_id);
void Hit_Player(int _n_id, int _p_id);
void Combat_Reward(int p_id, int n_id);
int distance_block(int a, int b);
bool isMovePossible(int _id, DIRECTION _direction);
bool isPeaceMonsterMovePossible(int _cid, int _mid, DIRECTION _direction);

bool isAllowAccess(int db_id, int cid);
void Save_UserInfo(int db_id, int c_id);

SECTOR GetSector(int _race, int _id);
void SetSector(int _race, int _id);

string Notice_Attack(int _attackID, int _targetID);

enum EVENT_TYPE { EV_MOVE, EV_HEAL, EV_ATTACK};
enum SESSION_STATE { ST_FREE, ST_ACCEPTED, ST_INGAME, ST_ACTIVE, ST_SLEEP};
enum COMP_TYPE { OP_ACCEPT, OP_RECV, OP_SEND, OP_RANDOM_MOVE };

vector<int> ConnectedPlayer;

struct TIMER_EVENT {
	int object_id;
	EVENT_TYPE ev;
	COMP_TYPE comp_type;
	chrono::system_clock::time_point act_time;
	int target_id;

	constexpr bool operator < (const TIMER_EVENT& _Left) const
	{
		return (act_time > _Left.act_time);
	}
};

priority_queue<TIMER_EVENT> timer_queue;
mutex timer_l;

class OVER_EXP {
public:
	WSAOVERLAPPED _over;
	WSABUF _wsabuf;
	char _send_buf[BUF_SIZE];
	COMP_TYPE _comp_type;
	int target_id;
	OVER_EXP()
	{
		_wsabuf.len = BUF_SIZE;
		_wsabuf.buf = _send_buf;
		_comp_type = OP_RECV;
		ZeroMemory(&_over, sizeof(_over));
	}
	OVER_EXP(char* packet)
	{
		_wsabuf.len = packet[0];
		_wsabuf.buf = _send_buf;
		ZeroMemory(&_over, sizeof(_over));
		_comp_type = OP_SEND;
		memcpy(_send_buf, packet, packet[0]);
	}
};

struct OBJ_STAT
{
	int		_id;
	int		_db_id;
	short	x, y;
	char	_name[NAME_SIZE];
	//RACE	race;
	int		race;
	short	level;
	int		exp, maxexp;
	int		hp, hpmax;
	SECTOR  sector;
	bool	isDead;
};

struct BLOCK {
	int     blockID;
	short	x, y;
	SECTOR  sector;
};

class SESSION {
	OVER_EXP _recv_over;

public:
	// lock
	mutex	_lock;
	mutex	_ViewListLock;

	SESSION_STATE _s_state;
	EVENT_TYPE _ev;
	SOCKET _socket;
	OBJ_STAT _obj_stat;
	unordered_set <int> view_list;
	unordered_set <int> block_view_list;

	vector<int> my_party;

	ATTACKTYPE _attacktype;
	MOVETYPE _movetype;
	chrono::system_clock::time_point next_move_time;
	int		_prev_remain;

public:
	SESSION()
	{
		_obj_stat._id = -1;
		_socket = 0;
		_obj_stat.x = rand() % W_WIDTH;
		_obj_stat.y = rand() % W_HEIGHT;
		_obj_stat._name[0] = 0;
		_obj_stat.level = 1;
		_obj_stat.exp = 0;
		_obj_stat.maxexp = _obj_stat.level * 100;
		_obj_stat.hpmax = _obj_stat.level * 100;
		_obj_stat.hp = _obj_stat.hpmax;
		_obj_stat.race = RACE::RACE_END;
		_s_state = ST_FREE;
	}

	~SESSION() {}

	void do_recv()
	{
		DWORD recv_flag = 0;
		memset(&_recv_over._over, 0, sizeof(_recv_over._over));
		_recv_over._wsabuf.len = BUF_SIZE - _prev_remain;
		_recv_over._wsabuf.buf = _recv_over._send_buf + _prev_remain;
		WSARecv(_socket, &_recv_over._wsabuf, 1, 0, &recv_flag,
			&_recv_over._over, 0);
	}

	void do_send(void* packet)
	{
		OVER_EXP* sdata = new OVER_EXP{ reinterpret_cast<char*>(packet) };
		WSASend(_socket, &sdata->_wsabuf, 1, 0, 0, &sdata->_over, 0);
	}

	void send_login_ok_packet(int c_id);
	void send_move_packet(int c_id, int client_time);
	void send_add_object(int c_id);
	void send_add_block(int _id);
	void send_chat_packet(int c_id, const char* mess);
	void Send_Remove_Packet(int c_id);
	void Send_Remove_Block(int _id);
	void Send_StatChange_Packet(int c_id, int n_id);
	void Send_PlayerAttack_Packet(int c_id);
	void Send_Notice_Packet(char _message[BUF_SIZE]);
};

array<SESSION, MAX_USER + NUM_NPC> clients;
array<BLOCK, NUM_BLOCK> blocks;
HANDLE g_h_iocp;
SOCKET g_s_socket;

void add_timer(int obj_id, int act_time, COMP_TYPE e_type, int target_id)
{
	using namespace chrono;
	TIMER_EVENT ev;
	ev.act_time = system_clock::now() + milliseconds(act_time);
	ev.object_id = obj_id;
	ev.comp_type = e_type;
	ev.target_id = target_id;
	timer_queue.push(ev);
}

void activate_npc(int id)
{
	clients[id]._s_state = ST_ACTIVE;
	//SESSION_STATE old_status = ST_SLEEP;
	//if (true == atomic_compare_exchange_strong(&clients[id]._s_state, &old_status, ST_ACTIVE))
	add_timer(id, 1000, OP_RANDOM_MOVE, 0);
}

int distance(int a, int b)
{
	return abs(clients[a]._obj_stat.x - clients[b]._obj_stat.x)
		+ abs(clients[a]._obj_stat.y - clients[b]._obj_stat.y);
}

int distance_block(int a, int b)
{
	return abs(clients[a]._obj_stat.x - blocks[b].x)
		+ abs(clients[a]._obj_stat.y - blocks[b].y);
}

bool isMovePossible(int _id, DIRECTION _direction)
{
	//clients[_id]._ViewListLock.lock();
	unordered_set<int> block_vl = clients[_id].block_view_list;
	unordered_set<int> obj_vl = clients[_id].view_list;
	//clients[_id]._ViewListLock.unlock();
	
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

bool isPeaceMonsterMovePossible(int _cid, int _mid, DIRECTION _direction)
{
	//clients[_cid]._ViewListLock.lock();
	unordered_set<int> block_vl = clients[_cid].block_view_list;
	//clients[_cid]._ViewListLock.unlock();

	unordered_set<int> obj_vl = clients[_cid].view_list;
	//clients[_id]._ViewListLock.unlock();

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

bool isAllowAccess(int db_id, int cid)
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

void Save_UserInfo(int db_id, int c_id)
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

SECTOR GetSector(int _race, int _id)
{
	if (_race == RACE_BLOCK) 
		return blocks[_id].sector;
	else 
		return clients[_id]._obj_stat.sector;
	
	return SECTOR_END;
}

void SetSector(int _race, int _id)
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

void SESSION::send_login_ok_packet(int c_id)
{
	SC_LOGIN_OK_PACKET p;
	p.size = sizeof(SC_LOGIN_OK_PACKET);
	p.type = SC_LOGIN_OK;
	///////////////DB에 있다면 DB에서 꺼내올 것들 //////////////////
	p.id = clients[c_id]._obj_stat._id;

	if (!isStressTest) {
		p.x = _obj_stat.x;
		p.y = _obj_stat.y;
		p.level = _obj_stat.level;
		p.exp = _obj_stat.exp;
		p.hpmax = _obj_stat.hpmax;
		p.hp = _obj_stat.hp;
		p.race = RACE::RACE_PLAYER;
	}

	if (isStressTest) {
		p.x = rand() % W_WIDTH;
		p.y = rand() % W_HEIGHT;

		p.level = 1;
		p.exp = 100;
		p.hpmax = 100;
		p.hp = 100;
		p.race = RACE::RACE_PLAYER;

		clients[p.id]._obj_stat.x = p.x;
		clients[p.id]._obj_stat.y = p.y;
	}

	////////////////////////////////////////////////////////////////

	SetSector(RACE_PLAYER, p.id);
	do_send(&p);
}

void SESSION::send_move_packet(int c_id, int client_time)
{
	SC_MOVE_OBJECT_PACKET p;
	p.id = c_id;
	p.size = sizeof(SC_MOVE_OBJECT_PACKET);
	p.type = SC_MOVE_OBJECT;
	p.x = clients[c_id]._obj_stat.x;
	p.y = clients[c_id]._obj_stat.y;
	p.client_time = client_time;
	do_send(&p);
}

void SESSION::send_add_object(int c_id)
{
	SC_ADD_OBJECT_PACKET p;
	p.id = c_id;
	p.size = sizeof(SC_ADD_OBJECT_PACKET);
	p.type = SC_ADD_OBJECT;
	p.x = clients[c_id]._obj_stat.x;
	p.y = clients[c_id]._obj_stat.y;
	p.race = clients[c_id]._obj_stat.race;
	p.level = clients[c_id]._obj_stat.level;
	p.hp = clients[c_id]._obj_stat.hp;
	p.hpmax = clients[c_id]._obj_stat.hpmax;
	strcpy_s(p.name, clients[c_id]._obj_stat._name);
	do_send(&p);
}

void SESSION::send_add_block(int _id)
{
	SC_ADD_OBJECT_PACKET p;
	p.id = _id;
	p.size = sizeof(SC_ADD_OBJECT_PACKET);
	p.type = SC_ADD_OBJECT;
	p.x = blocks[_id].x;
	p.y = blocks[_id].y;
	p.race = RACE_BLOCK;
	do_send(&p);
}

void SESSION::send_chat_packet(int c_id, const char *mess)
{
	SC_CHAT_PACKET p;
	p.id = c_id;
	p.size = sizeof(SC_CHAT_PACKET) - sizeof(p.mess) + strlen(mess) + 1;
	p.type = SC_CHAT;
	strcpy_s(p.mess, mess);
	do_send(&p);
}

void SESSION::Send_Remove_Packet(int c_id)
{
	SC_REMOVE_OBJECT_PACKET p;
	p.id = c_id;
	p.size = sizeof(p);
	p.type = SC_REMOVE_OBJECT;
	p.race = clients[c_id]._obj_stat.race;
	do_send(&p);
}

void SESSION::Send_Remove_Block(int _id)
{
	SC_REMOVE_OBJECT_PACKET p;
	p.id = _id;
	p.size = sizeof(p);
	p.type = SC_REMOVE_OBJECT;
	p.race = RACE_BLOCK;
	do_send(&p);
}

void SESSION::Send_StatChange_Packet(int c_id, int n_id)
{
	SC_STAT_CHANGE_PACKET packet;
	packet.size = sizeof(SC_STAT_CHANGE_PACKET);
	packet.type = SC_STAT_CHANGE;
	packet.hp = clients[n_id]._obj_stat.hp;
	packet.hpmax = clients[n_id]._obj_stat.hpmax;
	packet.id = n_id;
	packet.level = clients[n_id]._obj_stat.level;

	clients[c_id].do_send(&packet);
}

void SESSION::Send_PlayerAttack_Packet(int c_id)
{
	SC_PLAYER_ATTACK_PACKET packet;
	packet.size = sizeof(SC_PLAYER_ATTACK_PACKET);
	packet.type = SC_PLAYER_ATTACK;
	packet.id = c_id;

	do_send(&packet);
}

void SESSION::Send_Notice_Packet(char _message[BUF_SIZE])
{
	SC_CHAT_PACKET chat_packet;
	chat_packet.size = sizeof(chat_packet) - sizeof(chat_packet.mess) + strlen(_message) + 1;
	chat_packet.type = SC_CHAT;
	chat_packet.chat_type = CHATTYPE_NOTICE;
	chat_packet.id = 9999;
	strcpy_s(chat_packet.mess, _message);

	do_send(&chat_packet);
}

void disconnect(int c_id);

int get_new_client_id()
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

void process_packet(int c_id, char* packet)
{
	switch (packet[1]) {
	case CS_LOGIN: {
		CS_LOGIN_PACKET* p = reinterpret_cast<CS_LOGIN_PACKET*>(packet);

		if (p->name[0] == '\0')
			break;

		//if (!isStressTest)
			//isAllowAccess(p->db_id, c_id);

		clients[c_id]._lock.lock();
		strcpy_s(clients[c_id]._obj_stat._name, p->name);
		clients[c_id]._obj_stat._id = c_id;
		clients[c_id]._obj_stat._db_id = p->db_id;
		clients[c_id]._obj_stat.race = RACE::RACE_PLAYER;
		clients[c_id].send_login_ok_packet(c_id);
		clients[c_id]._s_state = ST_INGAME;
		clients[c_id]._lock.unlock();

		ConnectedPlayer.push_back(c_id);

		for (int i = 0; i < MAX_USER; ++i) {
			auto& pl = clients[i];
			pl._lock.lock();
			if (pl._obj_stat._id == c_id) {
				pl._lock.unlock();
				continue;
			}
			if (ST_INGAME != pl._s_state) {
				pl._lock.unlock();
				continue;
			}

			// 나중엔 거리가 가까운애들만 추가
			if (RANGE >= distance(c_id, pl._obj_stat._id)) {
				pl._ViewListLock.lock();
				pl.view_list.insert(c_id);
				pl._ViewListLock.unlock();
				pl.send_add_object(c_id);
			}		
			pl._lock.unlock();
		}

		for (auto& obj : clients) {
			if (obj._obj_stat._id == c_id) continue;
			//if (ST_INGAME != obj._s_state) continue;
			if (ST_FREE == obj._s_state) continue;

			if (RANGE >= distance(obj._obj_stat._id, c_id)) {
				clients[c_id]._ViewListLock.lock();
				clients[c_id].view_list.insert(obj._obj_stat._id);
				clients[c_id]._ViewListLock.unlock();
				clients[c_id].send_add_object(obj._obj_stat._id);
				obj._s_state = ST_INGAME;
			}
		}

		for (auto& block : blocks) {
			if (GetSector(clients[c_id]._obj_stat.race, c_id) != block.sector) continue;
			if (RANGE > distance_block(c_id, block.blockID))
				clients[c_id].send_add_block(block.blockID);
			
		}
		break;
	}
	case CS_MOVE: {
		// TODO 시야처리
		// LOCK FREE
		CS_MOVE_PACKET* p = reinterpret_cast<CS_MOVE_PACKET*>(packet);
		short x = clients[c_id]._obj_stat.x;
		short y = clients[c_id]._obj_stat.y;

		switch (p->direction) {
		case 0: 
			if (isMovePossible(c_id, DIRECTION_UP) && y > 0) y--; break;
		case 1: 
			if (isMovePossible(c_id, DIRECTION_DOWN) && y < W_HEIGHT - 1) y++; break;
		case 2:
			if (isMovePossible(c_id, DIRECTION_LEFT) && x > 0) x--; break;
		case 3:
			if (isMovePossible(c_id, DIRECTION_RIGHT) && x < W_WIDTH - 1) x++; break;
		}
			
		clients[c_id]._obj_stat.x = x;
		clients[c_id]._obj_stat.y = y;

		// 깨워주는 코드 부하가 엄청날듯
		for (int i = 0; i < MAX_USER + NUM_NPC; ++i) {
			if (clients[i]._obj_stat.sector != clients[c_id]._obj_stat.sector) continue;
			if (c_id == clients[i]._obj_stat._id) continue;

			if (RANGE > distance(c_id, i)) {
				if (clients[i]._obj_stat.race == RACE_END ||
					clients[i]._obj_stat.race == RACE_BLOCK ||					
					clients[i]._obj_stat.isDead) continue;
				clients[i]._s_state = ST_INGAME;
			}
		}
		///////////////////////////////////////////

		SetSector(clients[c_id]._obj_stat.race, c_id);

		clients[c_id]._ViewListLock.lock();
		unordered_set<int> old_vl = clients[c_id].view_list;
		clients[c_id]._ViewListLock.unlock();

		unordered_set<int> new_vl;
		for (int i = 0; i < MAX_USER + NUM_NPC; ++i) {
			if (clients[i]._obj_stat.sector != clients[c_id]._obj_stat.sector) continue;
			if (ST_INGAME != clients[i]._s_state) continue;
			//if (ST_FREE == clients[i]._s_state) continue;
			//if (ST_SLEEP == clients[i]._s_state) continue;
			if (c_id == clients[i]._obj_stat._id) continue;

			if (RANGE > distance(c_id, i))
				new_vl.insert(i);
		}
		clients[c_id].send_move_packet(c_id, 0);

		for (auto pl : new_vl) {
			// old_vl에 없는데 new_vl에 있으면 add패킷 보내기
			if (0 == old_vl.count(pl)) {
				clients[c_id].send_add_object(pl);
				clients[c_id]._ViewListLock.lock();
				clients[c_id].view_list.insert(pl);
				clients[c_id]._ViewListLock.unlock();
				clients[pl]._s_state = ST_INGAME;
				
				if (clients[pl]._obj_stat.race != RACE::RACE_PLAYER)	// player가 아니라면 패킷을 보낼 필요가 없다
					continue;
				clients[pl]._ViewListLock.lock();
				if (0 == clients[pl].view_list.count(c_id)) {
					clients[pl].send_add_object(c_id);
					clients[pl].view_list.insert(c_id);
					clients[pl]._ViewListLock.unlock();
					clients[pl]._s_state = ST_INGAME;
				}
				else {
					clients[pl]._ViewListLock.unlock();
					clients[pl].send_move_packet(c_id, 0);
				}
			}
			// old_vl에도 있고 new_vl에도 있으면 move패킷 보내기
			else {
				if (clients[pl]._obj_stat.race != RACE::RACE_PLAYER)	// player가 아니라면 패킷을 보낼 필요가 없다
					continue;

				clients[pl]._ViewListLock.lock();
				if (0 == clients[pl].view_list.count(c_id)) {
					clients[pl].send_add_object(c_id);
					clients[pl].view_list.insert(c_id);
					clients[pl]._ViewListLock.unlock();
					clients[pl]._s_state = ST_INGAME;
				}
				else {
					clients[pl]._ViewListLock.unlock();
					clients[pl].send_move_packet(c_id, 0);
				}
			}
		}

		for (auto pl : old_vl) {
			// old에는 있는데 new에는 없으면 삭제
			if (0 == new_vl.count(pl)) {
				clients[c_id].Send_Remove_Packet(pl);
				clients[c_id]._ViewListLock.lock();
				clients[c_id].view_list.erase(pl);
				clients[c_id]._ViewListLock.unlock();
				clients[pl]._s_state = ST_SLEEP;

				if (clients[pl]._obj_stat.race != RACE::RACE_PLAYER)	// player가 아니라면 패킷을 보낼 필요가 없다
					continue;
				clients[pl]._ViewListLock.lock();
				if (0 == clients[pl].view_list.count(c_id)) {
					clients[pl]._ViewListLock.unlock();
				}
				else {
					clients[pl].Send_Remove_Packet(c_id);
					clients[pl].view_list.erase(c_id);
					clients[pl]._ViewListLock.unlock();
					clients[pl]._s_state = ST_SLEEP;
				}
			}
		}

		// Block 시야 처리
		// 부하가 많으면 나중에 섹터로 나눠야 함
		for (int i = 0; i < NUM_BLOCK; ++i) {
			if (clients[c_id]._obj_stat.sector != blocks[i].sector) continue;
			if (RANGE > distance_block(c_id, i)) {
				clients[c_id]._ViewListLock.lock();
				clients[c_id].block_view_list.insert(i);
				clients[c_id]._ViewListLock.unlock();
				clients[c_id].send_add_block(i);
			}
			else {
				clients[c_id]._ViewListLock.lock();
				if (0 != clients[c_id].block_view_list.count(i)) {
					clients[c_id].block_view_list.erase(i);
					clients[c_id].Send_Remove_Block(i);
				}
				clients[c_id]._ViewListLock.unlock();			
			}
		}
		// move할때마다 저장하면 성능 안좋음 다른 방법 찾아야함
		if (!isStressTest)
			Save_UserInfo(clients[c_id]._obj_stat._db_id, c_id);
		break;
	}
	case CS_ATTACK: {
		for (auto& obj : clients[c_id].view_list) {
			if (clients[obj]._s_state == ST_SLEEP) continue;					// 몬스터가 이미 죽었으면 공격 불가
			clients[c_id].Send_PlayerAttack_Packet(c_id);
			if (clients[obj]._obj_stat.race == RACE_PLAYER) {				// 플레이어끼리는 공격 불가
				clients[c_id].Send_PlayerAttack_Packet(c_id);
				clients[obj].Send_PlayerAttack_Packet(c_id);
				continue;
			}
			if (distance(c_id, obj) < 2) {										// 공격 성공
				if (clients[obj]._obj_stat.race == RACE_PLAYER) continue;				// 플레이어끼리는 공격 불가			
				Hit_NPC(c_id, obj);

				for (auto& pl : clients[c_id].view_list) {
					if (clients[pl]._obj_stat.race != RACE_PLAYER) continue;
					clients[pl].Send_StatChange_Packet(pl, obj);				// 공격한 플레이어 주변 플레이어들에게 결과 보냄
					clients[pl].Send_StatChange_Packet(c_id, obj);				// 공격한 플레이어에게 전투 결과 보냄
				}
			}
		}


		break;
	}
	case CS_CHAT:
	{
		CS_CHAT_PACKET* p = reinterpret_cast<CS_CHAT_PACKET*>(packet);

		SC_CHAT_PACKET chat_packet;
		chat_packet.size = sizeof(chat_packet) - sizeof(chat_packet.mess) + strlen(p->mess) + 1;
		chat_packet.type = SC_CHAT;
		chat_packet.chat_type = CHATTYPE_SAY;
		chat_packet.id = c_id;
		strcpy_s(chat_packet.mess, p->mess);

		for (int& connected_id : ConnectedPlayer)
			clients[connected_id].do_send(&chat_packet);

		cout << "[" << clients[c_id]._obj_stat._name << "] : " << p->mess << "\n";
		break;
	}
	case CS_PARTY_INVITE:
	{
		CS_PARTY_INVITE_PACKET* p = reinterpret_cast<CS_PARTY_INVITE_PACKET*>(packet);
		for (int& connected_id : ConnectedPlayer) {
			if (clients[connected_id]._obj_stat._id == p->master_id) continue;
			if (distance(connected_id, p->master_id) < 2)
			{
				SC_PARTY_INVITE_PACKET party_packet;
				party_packet.size = sizeof(SC_PARTY_INVITE_PACKET);
				party_packet.type = SC_PARTY_INVITE;
				party_packet.id = p->master_id;

				clients[connected_id].do_send(&party_packet);
			}

		}
		break;
	}
	case CS_PARTY:
	{
		CS_PARTY_PACKET* p = reinterpret_cast<CS_PARTY_PACKET*>(packet);
		if (true == p->allow) {
			clients[p->master_id].my_party.push_back(p->id);
			clients[p->id].my_party.push_back(p->master_id);

			SC_PARTY_PACKET send_p;
			send_p.type = SC_PARTY;
			send_p.size = sizeof(SC_PARTY_PACKET);
			send_p.id = p->master_id;

			clients[p->id].do_send(&send_p);

			SC_PARTY_PACKET send_p2;
			send_p2.type = SC_PARTY;
			send_p2.size = sizeof(SC_PARTY_PACKET);
			send_p2.id = p->id;
			clients[p->master_id].do_send(&send_p2);
		}
		break;
	}
	}

}

void disconnect(int c_id)
{
	clients[c_id]._lock.lock();
	if (clients[c_id]._s_state == ST_FREE) {
		clients[c_id]._lock.unlock();
		return;
	}
	closesocket(clients[c_id]._socket);
	clients[c_id]._s_state = ST_FREE;
	clients[c_id]._lock.unlock();

	for (auto& pl : clients) {
		if (pl._obj_stat._id == c_id) continue;
		pl._lock.lock();
		if (pl._s_state != ST_INGAME) {
			pl._lock.unlock();
			continue;
		}
		pl.Send_Remove_Packet(c_id);	
		pl._lock.unlock();
	}

}

void do_worker()
{
	while (true) {
		DWORD num_bytes;
		ULONG_PTR key;
		WSAOVERLAPPED* over = nullptr;
		BOOL ret = GetQueuedCompletionStatus(g_h_iocp, &num_bytes, &key, &over, INFINITE);
		OVER_EXP* ex_over = reinterpret_cast<OVER_EXP*>(over);
		int client_id = static_cast<int>(key);
		if (FALSE == ret) {
			if (ex_over->_comp_type == OP_ACCEPT) cout << "Accept Error";
			else {
				cout << "GQCS Error on client[" << key << "]\n";
				disconnect(static_cast<int>(key));
				if (ex_over->_comp_type == OP_SEND) delete ex_over;
				continue;
			}
		}

		switch (ex_over->_comp_type) {
		case OP_ACCEPT: {
			SOCKET c_socket = reinterpret_cast<SOCKET>(ex_over->_wsabuf.buf);
			int client_id = get_new_client_id();
			if (client_id != -1) {
				clients[client_id]._obj_stat.x = 0;
				clients[client_id]._obj_stat.y = 0;
				clients[client_id]._obj_stat._id = client_id;
				clients[client_id]._obj_stat._name[0] = 0;
				clients[client_id]._prev_remain = 0;
				clients[client_id]._socket = c_socket;
				CreateIoCompletionPort(reinterpret_cast<HANDLE>(c_socket),
					g_h_iocp, client_id, 0);
				clients[client_id].do_recv();
				c_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
			}
			else {
				cout << "Max user exceeded.\n";
			}
			ZeroMemory(&ex_over->_over, sizeof(ex_over->_over));
			ex_over->_wsabuf.buf = reinterpret_cast<CHAR*>(c_socket);
			int addr_size = sizeof(SOCKADDR_IN);
			AcceptEx(g_s_socket, c_socket, ex_over->_send_buf, 0, addr_size + 16, addr_size + 16, 0, &ex_over->_over);
			break;
		}
		case OP_RECV: {
			if (0 == num_bytes) disconnect(client_id);
			int remain_data = num_bytes + clients[key]._prev_remain;
			char* p = ex_over->_send_buf;
			while (remain_data > 0) {
				int packet_size = p[0];
				if (packet_size <= remain_data) {
					process_packet(static_cast<int>(key), p);
					p = p + packet_size;
					remain_data = remain_data - packet_size;
				}
				else break;
			}
			clients[key]._prev_remain = remain_data;
			if (remain_data > 0) {
				memcpy(ex_over->_send_buf, p, remain_data);
			}
			clients[key].do_recv();
			break;
		}
		case OP_SEND:
			if (0 == num_bytes) disconnect(client_id);
			delete ex_over;
			break;
		case OP_RANDOM_MOVE: {
			// 일단 그냥 ROMING
			int npc_id = static_cast<int>(key);
			Move_NPC(npc_id, ex_over->target_id);
			delete ex_over;
		}
						   break;
		} //SWITCH
	} // WHILE
}

void Move_NPC(int _npc_id, int _c_id)
{
	unordered_set<int> old_vl;
	for (int i = 0; i < MAX_USER; ++i) {
		if (clients[i]._obj_stat.sector != clients[_npc_id]._obj_stat.sector) continue;
		if (clients[i]._s_state != ST_INGAME) continue;
		if (distance(_npc_id, i) <= RANGE) old_vl.insert(i);
		
	}

	if (clients[_npc_id]._attacktype == ATTACKTYPE_PEACE)
		PathFinder_Peace(_npc_id, _c_id);
	if (clients[_npc_id]._attacktype == ATTACKTYPE_AGRO)
		PathFinder_Agro(_npc_id, _c_id);

	unordered_set<int> new_vl;
	for (int i = 0; i < MAX_USER; ++i) {
		if (clients[i]._s_state != ST_INGAME) continue;
		if (distance(_npc_id, i) <= RANGE) new_vl.insert(i);
	}

	for (auto p_id : new_vl) {
		//if (clients[p_id]._obj_stat.race != RACE::RACE_PLAYER) continue;
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
		//if (clients[p_id]._obj_stat.race != RACE::RACE_PLAYER) continue;
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

void PathFinder_Peace(int _npc_id, int _c_id)
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

void PathFinder_Agro(int _npc_id, int _c_id)
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

void Hit_NPC(int _p_id, int n_id)
{
	clients[n_id]._obj_stat.hp -= clients[_p_id]._obj_stat.level * 50;
	
	char temp[BUF_SIZE];
	strcpy_s(temp, Notice_Attack(_p_id, n_id).c_str());
	clients[_p_id].Send_Notice_Packet(temp);
	clients[_p_id].Send_StatChange_Packet(_p_id, n_id);	// 몬스터 공격하여 체력이 변하면 플레이어에게 전송
	for (auto& pl : clients[_p_id].view_list) {
		if (clients[pl]._obj_stat.race != RACE_PLAYER) continue;
		clients[pl].Send_StatChange_Packet(pl, n_id);	// 몬스터 처치하여 스탯이 변하면 근처 플레이어에게 전송
	}

	if (clients[n_id]._obj_stat.hp <= 0) {		// 몬스터 사망
		clients[n_id]._s_state = ST_SLEEP;
		clients[n_id]._obj_stat.isDead = true;
		clients[_p_id].Send_Remove_Packet(n_id);	// 공격을 한 플레이어에게 전송

		Combat_Reward(_p_id, n_id);
		clients[_p_id].Send_StatChange_Packet(_p_id, _p_id);	// 몬스터 처치하여 스탯이 변하면 플레이어에게 전송

		// 시야 안의 모든 플레이어들에게 REMOVE패킷 전송
		for (auto& pl : clients[_p_id].view_list) {
			if (clients[pl]._obj_stat.race != RACE_PLAYER) continue;
			clients[pl].Send_Remove_Packet(n_id);			// 근처 플레이어들에게도 전송
			clients[pl].Send_StatChange_Packet(pl, _p_id);	// 몬스터 처치하여 스탯이 변하면 근처 플레이어에게 전송	
		}
	}

}

void Hit_Player(int _n_id, int _p_id)
{
	int AttackPower = clients[_n_id]._obj_stat.level * 2;
	clients[_p_id]._obj_stat.hp -= AttackPower;
	
	char temp[BUF_SIZE];
	strcpy_s(temp, Notice_Attack(_n_id, _p_id).c_str());
	clients[_p_id].Send_Notice_Packet(temp);
	clients[_p_id].Send_StatChange_Packet(_p_id, _p_id);	
	for (auto& pl : clients[_p_id].view_list) {
		if (clients[pl]._obj_stat.race != RACE_PLAYER) continue;
		clients[pl].Send_StatChange_Packet(pl, _p_id);	// 몬스터 처치하여 스탯이 변하면 근처 플레이어에게 전송	
	}

}

string Notice_Attack(int _attackID, int _targetID)
{
	string notice;
	notice += clients[_attackID]._obj_stat._name;
	notice += " Attacks a ";
	notice += clients[_targetID]._obj_stat._name;
	notice += " / Player HP to ";
	notice += to_string(clients[_targetID]._obj_stat.hp);

	return notice;
}

void Combat_Reward(int p_id, int n_id)
{
	// 몬스터 처치 보상
	int rewardEXP = clients[n_id]._obj_stat.level * clients[n_id]._obj_stat.level * 2;
	if (clients[n_id]._attacktype == ATTACKTYPE_AGRO)
		clients[p_id]._obj_stat.exp += rewardEXP * 2;
	else
		clients[p_id]._obj_stat.exp += rewardEXP;
	

	if (clients[p_id]._obj_stat.exp > clients[p_id]._obj_stat.maxexp) {
		clients[p_id]._obj_stat.level += 1;
		clients[p_id]._obj_stat.hpmax = clients[p_id]._obj_stat.level * 100;
		clients[p_id]._obj_stat.hp = clients[p_id]._obj_stat.hpmax;
		clients[p_id]._obj_stat.maxexp = clients[p_id]._obj_stat.level * 100;
		clients[p_id]._obj_stat.exp = 0;
	}
}

void do_ai_ver_heat_beat()
{
	for (;;) {
		auto start_t = chrono::system_clock::now();

		for (int i = MAX_USER; i < MAX_USER + NUM_NPC; ++i)
		{
			if (clients[i]._s_state == ST_SLEEP) continue;
			for (auto& c_id : ConnectedPlayer)
			{
				if (clients[c_id]._obj_stat.sector != clients[i]._obj_stat.sector) continue;

				if (distance(i, c_id) < RANGE)
				{
					auto ex_over = new OVER_EXP;
					ex_over->_comp_type = OP_RANDOM_MOVE;
					ex_over->target_id = c_id;
					PostQueuedCompletionStatus(g_h_iocp, 1, i, &ex_over->_over);
				}
				else if (distance(i, c_id) < RANGE + 1)
				{
					clients[c_id]._ViewListLock.lock();
					clients[c_id].view_list.erase(i);
					clients[c_id]._ViewListLock.unlock();
					clients[c_id].Send_Remove_Packet(i);
				}
			}
			
						
		}		
		auto end_t = chrono::system_clock::now();
		auto ai_t = end_t - start_t;
		this_thread::sleep_until(start_t + chrono::seconds(1));
	}

}

void ShowError(SQLHANDLE hHandle, SQLSMALLINT hType, RETCODE RetCode)
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

void Init_npc()
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
		//clients[i]._obj_stat.x = rand() % 10;
		//clients[i]._obj_stat.y = rand() % 10;
		/*clients[i]._obj_stat.x = rand() % W_WIDTH;
		clients[i]._obj_stat.y = rand() % W_WIDTH;*/
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

void Init_Block()
{
	for (int i = 0; i < NUM_BLOCK; ++i) 
	{
		blocks[i].blockID = i;
		blocks[i].x = rand() % W_WIDTH;
		blocks[i].y = rand() % W_WIDTH;
		SetSector(RACE_BLOCK, i);
	}
}

void initialize_DB()
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

int main()
{
	Init_Block();
	Init_npc();
	initialize_DB();

	WSADATA WSAData;
	WSAStartup(MAKEWORD(2, 2), &WSAData);
	g_s_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	SOCKADDR_IN server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORT_NUM);
	server_addr.sin_addr.S_un.S_addr = INADDR_ANY;
	bind(g_s_socket, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr));
	listen(g_s_socket, SOMAXCONN);
	SOCKADDR_IN cl_addr;
	int addr_size = sizeof(cl_addr);
	int client_id = 0;

	g_h_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
	CreateIoCompletionPort(reinterpret_cast<HANDLE>(g_s_socket), g_h_iocp, 9999, 0);
	SOCKET c_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	OVER_EXP a_over;
	a_over._comp_type = OP_ACCEPT;
	a_over._wsabuf.buf = reinterpret_cast<CHAR *>(c_socket);
	AcceptEx(g_s_socket, c_socket, a_over._send_buf, 0, addr_size + 16, addr_size + 16, 0, &a_over._over);

	vector <thread> worker_threads;
	for (int i = 0; i < 6; ++i)
		worker_threads.emplace_back(do_worker);

	thread ai_thread{ do_ai_ver_heat_beat };
	ai_thread.join();
	
	for (auto& th : worker_threads)
		th.join();
	closesocket(g_s_socket);
	WSACleanup();
}
