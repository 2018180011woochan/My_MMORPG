//#include <iostream>
//#include <array>
//#include <WS2tcpip.h>
//#include <MSWSock.h>
//
//#include <thread>
//#include <vector>
//#include <mutex>
//#include <unordered_set>
//#include <queue>
//#include <chrono>
//#include <string>
//#include <atomic>
//
//#include <windows.h>  
//#include <locale.h>
//
//#define UNICODE  
//#include <sqlext.h>  
//
//constexpr int RANGE = 15;
//
//#pragma comment(lib, "WS2_32.lib")
//#pragma comment(lib, "MSWSock.lib")
//using namespace std;
//
//#include "protocol.h"
//
//void do_timer();
//
//enum EVENT_TYPE { EV_MOVE, EV_HEAL, EV_ATTACK };
//enum SESSION_STATE { ST_FREE, ST_ACCEPTED, ST_INGAME, ST_ACTIVE, ST_SLEEP };
//enum COMP_TYPE { OP_ACCEPT, OP_RECV, OP_SEND, OP_RANDOM_MOVE };
//
//vector<int> ConnectedPlayer;
//
//struct TIMER_EVENT {
//	int object_id;
//	EVENT_TYPE ev;
//	COMP_TYPE comp_type;
//	chrono::system_clock::time_point act_time;
//	int target_id;
//
//	constexpr bool operator < (const TIMER_EVENT& _Left) const
//	{
//		return (act_time > _Left.act_time);
//	}
//};
//
//priority_queue<TIMER_EVENT> timer_queue;
//mutex timer_l;
//
//class OVER_EXP {
//public:
//	WSAOVERLAPPED _over;
//	WSABUF _wsabuf;
//	char _send_buf[BUF_SIZE];
//	COMP_TYPE _comp_type;
//	int target_id;
//	OVER_EXP()
//	{
//		_wsabuf.len = BUF_SIZE;
//		_wsabuf.buf = _send_buf;
//		_comp_type = OP_RECV;
//		ZeroMemory(&_over, sizeof(_over));
//	}
//	OVER_EXP(char* packet)
//	{
//		_wsabuf.len = packet[0];
//		_wsabuf.buf = _send_buf;
//		ZeroMemory(&_over, sizeof(_over));
//		_comp_type = OP_SEND;
//		memcpy(_send_buf, packet, packet[0]);
//	}
//};
//
//class SESSION {
//	OVER_EXP _recv_over;
//
//public:
//	mutex	_sl;
//	SESSION_STATE _s_state;
//	EVENT_TYPE _ev;
//	SOCKET _socket;
//	int		_id;
//	short	x, y;
//	char	_name[NAME_SIZE];
//	short	race;
//	short	level;
//	int		exp, maxexp;
//	int		hp, hpmax;
//	unordered_set <int> view_list;
//	chrono::system_clock::time_point next_move_time;
//	int		_prev_remain;
//public:
//	SESSION()
//	{
//		_id = -1;
//		_socket = 0;
//		x = rand() % W_WIDTH;
//		y = rand() % W_HEIGHT;
//		_name[0] = 0;
//		level = 1;
//		exp = 0;
//		maxexp = level * 100;
//		hpmax = level * 100;
//		hp = hpmax;
//		_s_state = ST_FREE;
//		_prev_remain = 0;
//		next_move_time = chrono::system_clock::now() + chrono::seconds(1);
//	}
//
//	~SESSION() {}
//
//	void do_recv()
//	{
//		DWORD recv_flag = 0;
//		memset(&_recv_over._over, 0, sizeof(_recv_over._over));
//		_recv_over._wsabuf.len = BUF_SIZE - _prev_remain;
//		_recv_over._wsabuf.buf = _recv_over._send_buf + _prev_remain;
//		WSARecv(_socket, &_recv_over._wsabuf, 1, 0, &recv_flag,
//			&_recv_over._over, 0);
//	}
//
//	void do_send(void* packet)
//	{
//		OVER_EXP* sdata = new OVER_EXP{ reinterpret_cast<char*>(packet) };
//		WSASend(_socket, &sdata->_wsabuf, 1, 0, 0, &sdata->_over, 0);
//	}
//};
//
//array<SESSION, MAX_USER + NUM_NPC> clients;
//HANDLE g_h_iocp;
//SOCKET g_s_socket;
//
//void add_timer(int obj_id, int act_time, COMP_TYPE e_type, int target_id)
//{
//	using namespace chrono;
//	TIMER_EVENT ev;
//	ev.act_time = system_clock::now() + milliseconds(act_time);
//	ev.object_id = obj_id;
//	ev.comp_type = e_type;
//	ev.target_id = target_id;
//	timer_queue.push(ev);
//}
//
//void activate_npc(int id)
//{
//	clients[id]._s_state = ST_ACTIVE;
//	//SESSION_STATE old_status = ST_SLEEP;
//	//if (true == atomic_compare_exchange_strong(&clients[id]._s_state, &old_status, ST_ACTIVE))
//	add_timer(id, 1000, OP_RANDOM_MOVE, 0);
//}
//
//int distance(int a, int b)
//{
//	return abs(clients[a].x - clients[b].x) + abs(clients[a].y - clients[b].y);
//}
//
//void disconnect(int c_id);
//
//int get_new_client_id()
//{
//	for (int i = 0; i < MAX_USER; ++i) {
//		clients[i]._sl.lock();
//		if (clients[i]._s_state == ST_FREE) {
//			clients[i]._s_state = ST_ACCEPTED;
//			clients[i]._sl.unlock();
//			return i;
//		}
//		clients[i]._sl.unlock();
//	}
//	return -1;
//}
//
//void process_packet(int c_id, char* packet)
//{
//	switch (packet[1]) {
//	
//	}
//
//}
//
//void disconnect(int c_id)
//{
//	
//}
//
//void do_worker()
//{
//	while (true) {
//		DWORD num_bytes;
//		ULONG_PTR key;
//		WSAOVERLAPPED* over = nullptr;
//		BOOL ret = GetQueuedCompletionStatus(g_h_iocp, &num_bytes, &key, &over, INFINITE);
//		OVER_EXP* ex_over = reinterpret_cast<OVER_EXP*>(over);
//		int client_id = static_cast<int>(key);
//		if (FALSE == ret) {
//			if (ex_over->_comp_type == OP_ACCEPT) cout << "Accept Error";
//			else {
//				cout << "GQCS Error on client[" << key << "]\n";
//				disconnect(static_cast<int>(key));
//				if (ex_over->_comp_type == OP_SEND) delete ex_over;
//				continue;
//			}
//		}
//
//		switch (ex_over->_comp_type) {
//		case OP_ACCEPT: {
//			SOCKET c_socket = reinterpret_cast<SOCKET>(ex_over->_wsabuf.buf);
//			int client_id = get_new_client_id();
//			if (client_id != -1) {
//				clients[client_id].x = 0;
//				clients[client_id].y = 0;
//				clients[client_id]._id = client_id;
//				clients[client_id]._name[0] = 0;
//				clients[client_id]._prev_remain = 0;
//				clients[client_id]._socket = c_socket;
//				CreateIoCompletionPort(reinterpret_cast<HANDLE>(c_socket),
//					g_h_iocp, client_id, 0);
//				clients[client_id].do_recv();
//				c_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
//			}
//			else {
//				cout << "Max user exceeded.\n";
//			}
//			ZeroMemory(&ex_over->_over, sizeof(ex_over->_over));
//			ex_over->_wsabuf.buf = reinterpret_cast<CHAR*>(c_socket);
//			int addr_size = sizeof(SOCKADDR_IN);
//			AcceptEx(g_s_socket, c_socket, ex_over->_send_buf, 0, addr_size + 16, addr_size + 16, 0, &ex_over->_over);
//			break;
//		}
//		case OP_RECV: {
//			if (0 == num_bytes) disconnect(client_id);
//			int remain_data = num_bytes + clients[key]._prev_remain;
//			char* p = ex_over->_send_buf;
//			while (remain_data > 0) {
//				int packet_size = p[0];
//				if (packet_size <= remain_data) {
//					process_packet(static_cast<int>(key), p);
//					p = p + packet_size;
//					remain_data = remain_data - packet_size;
//				}
//				else break;
//			}
//			clients[key]._prev_remain = remain_data;
//			if (remain_data > 0) {
//				memcpy(ex_over->_send_buf, p, remain_data);
//			}
//			clients[key].do_recv();
//			break;
//		}
//		case OP_SEND:
//			if (0 == num_bytes) disconnect(client_id);
//			delete ex_over;
//			break;
//		} //SWITCH
//	} // WHILE
//}
//
//void ShowError(SQLHANDLE hHandle, SQLSMALLINT hType, RETCODE RetCode)
//{
//	SQLSMALLINT iRec = 0;
//	SQLINTEGER iError;
//	WCHAR wszMessage[1000];
//	WCHAR wszState[SQL_SQLSTATE_SIZE + 1];
//	if (RetCode == SQL_INVALID_HANDLE) {
//		fwprintf(stderr, L"Invalid handle!\n");
//		return;
//	}
//	while (SQLGetDiagRec(hType, hHandle, ++iRec, wszState, &iError, wszMessage,
//		(SQLSMALLINT)(sizeof(wszMessage) / sizeof(WCHAR)), (SQLSMALLINT*)NULL) == SQL_SUCCESS) {
//		// Hide data truncated..
//		if (wcsncmp(wszState, L"01004", 5)) {
//			fwprintf(stderr, L"[%5.5s] %s (%d)\n", wszState, wszMessage, iError);
//		}
//	}
//}
//
//void do_timer()
//{
//	while (true)
//	{
//		this_thread::sleep_for(1ms); //Sleep(1);
//
//		for (int i = 0; i < NUM_NPC; ++i) {
//			int npc_id = MAX_USER + i;
//			OVER_EXP* over = new OVER_EXP();
//			over->_comp_type = COMP_TYPE::OP_RANDOM_MOVE;
//			PostQueuedCompletionStatus(g_h_iocp, 1, npc_id, &over->_over);
//			//random_move_npc(ev.obj_id);
//			//add_timer(ev.obj_id, ev.event_id, 1000);
//		}
//	}
//}
//
//
//
//int main()
//{
//	WSADATA WSAData;
//	WSAStartup(MAKEWORD(2, 2), &WSAData);
//	g_s_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
//	SOCKADDR_IN server_addr;
//	memset(&server_addr, 0, sizeof(server_addr));
//	server_addr.sin_family = AF_INET;
//	server_addr.sin_port = htons(PORT_NUM);
//	server_addr.sin_addr.S_un.S_addr = INADDR_ANY;
//	bind(g_s_socket, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr));
//	listen(g_s_socket, SOMAXCONN);
//	SOCKADDR_IN cl_addr;
//	int addr_size = sizeof(cl_addr);
//	int client_id = 0;
//
//	ConnectedPlayer.reserve(10000);
//
//	g_h_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
//	CreateIoCompletionPort(reinterpret_cast<HANDLE>(g_s_socket), g_h_iocp, 9999, 0);
//	SOCKET c_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
//	OVER_EXP a_over;
//	a_over._comp_type = OP_ACCEPT;
//	a_over._wsabuf.buf = reinterpret_cast<CHAR*>(c_socket);
//	AcceptEx(g_s_socket, c_socket, a_over._send_buf, 0, addr_size + 16, addr_size + 16, 0, &a_over._over);
//
//	vector <thread> worker_threads;
//	for (int i = 0; i < 6; ++i)
//		worker_threads.emplace_back(do_worker);
//
//	for (auto& th : worker_threads)
//		th.join();
//	closesocket(g_s_socket);
//	WSACleanup();
//}

#include "pch.h"
#include <iostream>
#include "ThreadManager.h"
#include "memory.h"

#include "Service.h"
#include "Session.h"
#include "GameSession.h"
#include "GameSessionManager.h"
#include "BufferWriter.h"
#include "ServerPacketHandler.h"

#include "protocol.h"

int main()
{
	ServerServiceRef service = MakeShared<ServerService>(
		NetAddress(L"127.0.0.1", PORT_NUM),
		MakeShared<IocpCore>(),
		MakeShared<GameSession>,
		100 /*동접*/ );

	service->Start();
	// 이런식으로 서비스 만들어서 스타트 때리면 알아서 붙는다
	
	// 스레드 개수는 코어 개수~ 코어개수1.5배
	for (int32 i = 0; i < 5; ++i)
	{
		GThreadManager->Launch([=]()
			{
				while (true)
				{
					service->GetIocpCore()->Dispatch();
				}
			});
	}

	//while(true) 
	{
		/*vector<BuffData> buffs{ BuffData {100, 1.5f}, BuffData {200, 2.3f}, BuffData {300, 7.5f} };
		SendBufferRef sendBuffer = ServerPacketHandler::Make_S_TEST(1001, 100, 10, buffs, L"김우찬");*/

		//GameSessionRef player;
		//player->ClientID = GSessionManager.GetAcceptedID();
		//player->ClientID = 9999;

		//SendBufferRef sendBuffer = ServerPacketHandler::Make_SC_LOGIN(0, L"김우찬");

		//GSessionManager.Broadcast(sendBuffer);

		//this_thread::sleep_for(250ms);
	}

	GThreadManager->Join();
}