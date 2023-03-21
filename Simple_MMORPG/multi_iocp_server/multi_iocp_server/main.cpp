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

constexpr int RANGE = 7;

#pragma comment(lib, "WS2_32.lib")
#pragma comment(lib, "MSWSock.lib")
using namespace std;

void do_timer();
void Init_npc();

enum EVENT_TYPE { EV_MOVE, EV_HEAL, EV_ATTACK};
enum SESSION_STATE { ST_FREE, ST_ACCEPTED, ST_INGAME, ST_ACTIVE, ST_SLEEP};
enum COMP_TYPE { OP_ACCEPT, OP_RECV, OP_SEND, OP_RANDOM_MOVE };

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
	short	race;
	short	level;
	int		exp, maxexp;
	int		hp, hpmax;
	short	attack_type;
	short	move_type;
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
	void send_login_ok_packet()
	{
		SC_LOGIN_OK_PACKET p;
		p.size = sizeof(SC_LOGIN_OK_PACKET);
		p.type = SC_LOGIN_OK;
		///////////////DB에 있다면 DB에서 꺼내올 것들 //////////////////
		p.id = _obj_stat._id;
		p.x = _obj_stat.x;
		p.y = _obj_stat.y;
		p.level = _obj_stat.level;
		p.exp = _obj_stat.exp;
		p.hpmax = _obj_stat.hpmax;
		p.hp = p.hpmax;
		p.race = RACE::RACE_PLAYER;
		////////////////////////////////////////////////////////////////
		do_send(&p);
	}
	void send_move_packet(int c_id, int client_time);
	void send_add_object(int c_id);
	void send_chat_packet(int c_id, const char* mess);
	void Send_Remove_Packet(int c_id);
};

array<SESSION, MAX_USER + NUM_NPC> clients;
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
	do_send(&p);
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

		clients[c_id]._lock.lock();
		strcpy_s(clients[c_id]._obj_stat._name, p->name);
		clients[c_id]._obj_stat._id = c_id;
		clients[c_id]._obj_stat._db_id = p->db_id;
		clients[c_id].send_login_ok_packet();
		clients[c_id]._s_state = ST_INGAME;
		clients[c_id]._lock.unlock();

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
			if (ST_INGAME != obj._s_state) continue;

			if (RANGE >= distance(obj._obj_stat._id, c_id)) {
				clients[c_id]._ViewListLock.lock();
				clients[c_id].view_list.insert(obj._obj_stat._id);
				clients[c_id]._ViewListLock.unlock();
				clients[c_id].send_add_object(obj._obj_stat._id);
			}
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
		case 0: if (y > 0) y--; break;
		case 1: if (y < W_HEIGHT - 1) y++; break;
		case 2: if (x > 0) x--; break;
		case 3: if (x < W_WIDTH - 1) x++; break;
		}
			
		clients[c_id]._obj_stat.x = x;
		clients[c_id]._obj_stat.y = y;

		clients[c_id]._ViewListLock.lock();
		unordered_set<int> old_vl = clients[c_id].view_list;
		clients[c_id]._ViewListLock.unlock();

		unordered_set<int> new_vl;
		for (int i = 0; i < MAX_USER; ++i) {
			if (ST_INGAME != clients[i]._s_state) continue;
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

				clients[pl]._ViewListLock.lock();
				if (0 == clients[pl].view_list.count(c_id)) {
					clients[pl].send_add_object(c_id);
					clients[pl].view_list.insert(c_id);
					clients[pl]._ViewListLock.unlock();
				}
				else {
					clients[pl]._ViewListLock.unlock();
					clients[pl].send_move_packet(c_id, 0);
				}
			}
			// old_vl에도 있고 new_vl에도 있으면 move패킷 보내기
			else {
				clients[pl]._ViewListLock.lock();
				if (0 == clients[pl].view_list.count(c_id)) {
					clients[pl].send_add_object(c_id);
					clients[pl].view_list.insert(c_id);
					clients[pl]._ViewListLock.unlock();
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

				clients[pl]._ViewListLock.lock();
				if (0 == clients[pl].view_list.count(c_id)) {
					clients[pl]._ViewListLock.unlock();
				}
				else {
					clients[pl].Send_Remove_Packet(c_id);
					clients[pl].view_list.erase(c_id);
					clients[pl]._ViewListLock.unlock();
				}
			}
		}

//#pragma region NPC시야처리
//		unordered_set<int> Old_ViewList;
//		for (int i = 0; i < MAX_USER + NUM_NPC; ++i) {
//			if (ST_INGAME != clients[i]._s_state) continue;
//			if (RANGE > distance(c_id, i)) Old_ViewList.insert(i);
//		}
//
//		switch (p->direction) {
//		case 0: if (y > 0) y--; break;
//		case 1: if (y < W_HEIGHT - 1) y++; break;
//		case 2: if (x > 0) x--; break;
//		case 3: if (x < W_WIDTH - 1) x++; break;
//		}
//
//		clients[c_id]._obj_stat.x = x;
//		clients[c_id]._obj_stat.y = y;
//
//		//for (auto& pl : clients) {
//		//	pl._lock.lock();
//		//	if (ST_INGAME == pl._s_state)
//		//		pl.send_move_packet(c_id, p->client_time);
//		//	pl._lock.unlock();
//		//}
//
//		unordered_set<int> New_ViewList;
//		for (int i = 0; i < MAX_USER + NUM_NPC; ++i) {
//			if (ST_INGAME != clients[i]._s_state) continue;
//			if (distance(c_id, i)) New_ViewList.insert(i);
//		}
//
//		for (auto p_id : New_ViewList) {
//			// viewlist에 없는데 NewViewList에 있다 -> viewlist에 추가 후 add패킷 전송
//			if (0 == clients[c_id].view_list.count(p_id)) {
//				clients[c_id].view_list.insert(p_id);
//				clients[c_id].send_add_object(p_id);
//			}
//			// viewlist에 있고 NewViewList에도 있다 -> move패킷 전송
//			else
//				clients[c_id].send_move_packet(p_id, 0);
//		}
//		for (auto p_id : Old_ViewList) {
//			// old에는 있는데 new에는 없다 -> 시야 밖으로 나갔으므로 remove패킷 보냄
//			if (0 == New_ViewList.count(p_id)) {
//				if (1 == clients[c_id].view_list.count(p_id)) {
//					clients[c_id].view_list.erase(p_id);
//
//					clients[c_id].Send_Remove_Packet(p_id);
//				}
//			}
//		}
//#pragma endregion NPC시야처리
//
//#pragma region Player시야처리
//		/*for (int i = 0; i < MAX_USER; ++i) {
//			auto& pl = clients[i];
//			if (pl._obj_stat._id == c_id) continue;
//			pl._lock.lock();
//			if (ST_INGAME != pl._s_state) {
//				pl._lock.unlock();
//				continue;
//			}
//			if (RANGE >= distance(c_id, pl._obj_stat._id)) {
//				pl._ViewListLock.lock();
//				pl.view_list.insert(c_id);
//				pl._ViewListLock.unlock();
//				pl.send_add_object(c_id);
//			}
//			pl._lock.unlock();
//		}
//		for (auto& pl : clients) {
//			if (pl._obj_stat._id == c_id) continue;
//			pl._lock.lock();
//			if (ST_INGAME != pl._s_state) {
//				pl._lock.unlock();
//				continue;
//			}
//			if (RANGE >= distance(pl._obj_stat._id, c_id)) {
//				clients[c_id]._ViewListLock.lock();
//				clients[c_id].view_list.insert(pl._obj_stat._id);
//				clients[c_id]._ViewListLock.unlock();
//				clients[c_id].send_add_object(pl._obj_stat._id);
//			}
//			pl._lock.unlock();
//		}*/
//
//#pragma endregion Player시야처리

		break;
	}
	case CS_ATTACK: {

		break;
	}
	case CS_CHAT:
	{
		
		break;
	}
	case CS_PARTY_INVITE:
	{
		
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

			delete ex_over;
		}
						   break;
		} //SWITCH
	} // WHILE
}


void do_ai_ver_heat_beat()
{
	for (;;) {
		auto start_t = chrono::system_clock::now();

		for (int i = 10000; i < NUM_NPC; ++i)
		{

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

void do_timer()
{
	while (true)
	{
		this_thread::sleep_for(1ms); //Sleep(1);

		for (int i = 0; i < NUM_NPC; ++i) {
			int npc_id = MAX_USER + i;
			OVER_EXP* over = new OVER_EXP();
			over->_comp_type = COMP_TYPE::OP_RANDOM_MOVE;
			PostQueuedCompletionStatus(g_h_iocp, 1, npc_id, &over->_over);
			//random_move_npc(ev.obj_id);
			//add_timer(ev.obj_id, ev.event_id, 1000);
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
		clients[i]._s_state = ST_INGAME;
		clients[i]._obj_stat.race = RACE::RACE_SKELETON;
		clients[i]._obj_stat.level = 1;
		clients[i]._obj_stat.hpmax = clients[i]._obj_stat.level * 100;
		clients[i]._obj_stat.hp = clients[i]._obj_stat.hpmax;
		clients[i]._obj_stat.move_type = MOVETYPE::MOVETYPE_FIX;
		clients[i]._obj_stat.attack_type = ATTACKTYPE::ATTACKTYPE_PEACE;
		strcpy_s(clients[i]._obj_stat._name, "Skeleton");
	}
	for (int i = MAX_USER + 50000; i < MAX_USER + 100000; ++i)
	{
		// Wraith
		clients[i]._s_state = ST_INGAME;
		clients[i]._obj_stat.race = RACE::RACE_WRIATH;
		clients[i]._obj_stat.level = 2;
		clients[i]._obj_stat.hpmax = clients[i]._obj_stat.level * 100;
		clients[i]._obj_stat.hp = clients[i]._obj_stat.hpmax;
		clients[i]._obj_stat.move_type = MOVETYPE::MOVETYPE_FIX;
		clients[i]._obj_stat.attack_type = ATTACKTYPE::ATTACKTYPE_AGRO;
		strcpy_s(clients[i]._obj_stat._name, "Wriath");
	}
	for (int i = MAX_USER + 100000; i < MAX_USER + 150000; ++i)
	{
		// Devil
		clients[i]._s_state = ST_INGAME;
		clients[i]._obj_stat.race = RACE::RACE_DEVIL;
		clients[i]._obj_stat.level = 3;
		clients[i]._obj_stat.hpmax = clients[i]._obj_stat.level * 100;
		clients[i]._obj_stat.hp = clients[i]._obj_stat.hpmax;
		clients[i]._obj_stat.move_type = MOVETYPE::MOVETYPE_ROAMING;
		clients[i]._obj_stat.attack_type = ATTACKTYPE::ATTACKTYPE_PEACE;
		strcpy_s(clients[i]._obj_stat._name, "Devil");
	}
	
}

int main()
{
	//Init_npc();

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
