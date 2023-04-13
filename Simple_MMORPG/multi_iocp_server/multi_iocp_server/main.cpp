#include "Include.h"
#include "SessionManager.h"

// 해야댈거
// 로그인 실패 만들기 O
// 죽었을때 시야처리 잘 안되는거 수정 O
// 파티 경험치 공유 O
// 플레이어 5초마다 HP회복
// 몬스터 사망후 30초후 부활
// 무찔렀을 경우 경험치 : 레벨 * 레벨 * 2 O
// 어그로, 로밍 2배 O
// 공격속도 1초에 한번
// 몬스터 잡았을때 경험치 메시지 O

HANDLE g_h_iocp;
SOCKET g_s_socket;

void disconnect(int c_id);

void process_packet(int c_id, char* packet)
{
	switch (packet[1]) {
	case CS_LOGIN: {
		CS_LOGIN_PACKET* p = reinterpret_cast<CS_LOGIN_PACKET*>(packet);

		if (p->name[0] == '\0')
			break;

		for (int i = 0; i < MAX_USER; ++i)
		{
			if (GSessionManager.clients[i]._obj_stat._db_id == p->db_id)
			{
				if (GSessionManager.clients[i]._s_state == ST_INGAME)
				{
					// loginfail보내기
					SC_LOGIN_FAIL_PACKET login_fail_packet;
					login_fail_packet.size = sizeof(SC_LOGIN_FAIL_PACKET);
					login_fail_packet.type = SC_LOGIN_FAIL;
					login_fail_packet.reason = 1;
					GSessionManager.clients[c_id].do_send(&login_fail_packet);
					return;
				}
			}
		}

		GSessionManager.isAllowAccess(p->db_id, c_id);

		GSessionManager.clients[c_id]._lock.lock();
		strcpy_s(GSessionManager.clients[c_id]._obj_stat._name, p->name);
		GSessionManager.clients[c_id]._obj_stat._id = c_id;
		GSessionManager.clients[c_id]._obj_stat._db_id = p->db_id;
		GSessionManager.clients[c_id]._obj_stat.race = RACE::RACE_PLAYER;
		GSessionManager.clients[c_id].send_login_ok_packet(c_id);
		GSessionManager.clients[c_id]._s_state = ST_INGAME;
		GSessionManager.clients[c_id]._lock.unlock();

		GSessionManager.ConnectedPlayer.push_back(c_id);

		for (int i = 0; i < MAX_USER; ++i) {
			auto& pl = GSessionManager.clients[i];
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
			if (RANGE >= GSessionManager.distance(c_id, pl._obj_stat._id)) {
				pl._ViewListLock.lock();
				pl.view_list.insert(c_id);
				pl._ViewListLock.unlock();
				pl.send_add_object(c_id);
			}
			pl._lock.unlock();
		}

		for (auto& obj : GSessionManager.clients) {
			if (obj._obj_stat._id == c_id) continue;
			//if (ST_INGAME != obj._s_state) continue;
			if (ST_FREE == obj._s_state) continue;

			if (RANGE >= GSessionManager.distance(obj._obj_stat._id, c_id)) {
				GSessionManager.clients[c_id]._ViewListLock.lock();
				GSessionManager.clients[c_id].view_list.insert(obj._obj_stat._id);
				GSessionManager.clients[c_id]._ViewListLock.unlock();
				GSessionManager.clients[c_id].send_add_object(obj._obj_stat._id);
				obj._s_state = ST_INGAME;
			}
		}

		for (auto& block : GSessionManager.blocks) {
			if (GSessionManager.GetSector(GSessionManager.clients[c_id]._obj_stat.race, c_id) != block.sector) continue;
			if (RANGE > GSessionManager.distance_block(c_id, block.blockID))
				GSessionManager.clients[c_id].send_add_block(block.blockID);

		}
		break;
	}
	case CS_MOVE: {
		// TODO 시야처리
		// LOCK FREE
		CS_MOVE_PACKET* p = reinterpret_cast<CS_MOVE_PACKET*>(packet);
		short x = GSessionManager.clients[c_id]._obj_stat.x;
		short y = GSessionManager.clients[c_id]._obj_stat.y;

		switch (p->direction) {
		case 0:
			if (GSessionManager.isMovePossible(c_id, DIRECTION_UP) && y > 0) y--; break;
		case 1:
			if (GSessionManager.isMovePossible(c_id, DIRECTION_DOWN) && y < W_HEIGHT - 1) y++; break;
		case 2:
			if (GSessionManager.isMovePossible(c_id, DIRECTION_LEFT) && x > 0) x--; break;
		case 3:
			if (GSessionManager.isMovePossible(c_id, DIRECTION_RIGHT) && x < W_WIDTH - 1) x++; break;
		}

		GSessionManager.clients[c_id]._obj_stat.x = x;
		GSessionManager.clients[c_id]._obj_stat.y = y;

		// 깨워주는 코드 부하가 엄청날듯
		for (int i = 0; i < MAX_USER + NUM_NPC; ++i) {
			if (GSessionManager.clients[i]._obj_stat.sector != GSessionManager.clients[c_id]._obj_stat.sector) continue;
			if (c_id == GSessionManager.clients[i]._obj_stat._id) continue;

			if (RANGE > GSessionManager.distance(c_id, i)) {
				if (GSessionManager.clients[i]._obj_stat.race == RACE_END ||
					GSessionManager.clients[i]._obj_stat.race == RACE_BLOCK ||
					GSessionManager.clients[i]._obj_stat.isDead) continue;
				GSessionManager.clients[i]._s_state = ST_INGAME;
			}
		}
		///////////////////////////////////////////

		GSessionManager.SetSector(GSessionManager.clients[c_id]._obj_stat.race, c_id);

		GSessionManager.clients[c_id]._ViewListLock.lock();
		unordered_set<int> old_vl = GSessionManager.clients[c_id].view_list;
		GSessionManager.clients[c_id]._ViewListLock.unlock();

		unordered_set<int> new_vl;
		for (int i = 0; i < MAX_USER + NUM_NPC; ++i) {
			if (GSessionManager.clients[i]._obj_stat.sector != GSessionManager.clients[c_id]._obj_stat.sector) continue;
			if (ST_INGAME != GSessionManager.clients[i]._s_state) continue;
			//if (ST_FREE == GSessionManager.clients[i]._s_state) continue;
			//if (ST_SLEEP == GSessionManager.clients[i]._s_state) continue;
			if (c_id == GSessionManager.clients[i]._obj_stat._id) continue;

			if (RANGE > GSessionManager.distance(c_id, i))
				new_vl.insert(i);
		}
		GSessionManager.clients[c_id].send_move_packet(c_id, 0);

		for (auto pl : new_vl) {
			// old_vl에 없는데 new_vl에 있으면 add패킷 보내기
			if (0 == old_vl.count(pl)) {
				GSessionManager.clients[c_id].send_add_object(pl);
				GSessionManager.clients[c_id]._ViewListLock.lock();
				GSessionManager.clients[c_id].view_list.insert(pl);
				GSessionManager.clients[c_id]._ViewListLock.unlock();
				GSessionManager.clients[pl]._s_state = ST_INGAME;

				if (GSessionManager.clients[pl]._obj_stat.race != RACE::RACE_PLAYER)	// player가 아니라면 패킷을 보낼 필요가 없다
					continue;
				GSessionManager.clients[pl]._ViewListLock.lock();
				if (0 == GSessionManager.clients[pl].view_list.count(c_id)) {
					GSessionManager.clients[pl].send_add_object(c_id);
					GSessionManager.clients[pl].view_list.insert(c_id);
					GSessionManager.clients[pl]._ViewListLock.unlock();
					GSessionManager.clients[pl]._s_state = ST_INGAME;
				}
				else {
					GSessionManager.clients[pl]._ViewListLock.unlock();
					GSessionManager.clients[pl].send_move_packet(c_id, 0);
				}
			}
			// old_vl에도 있고 new_vl에도 있으면 move패킷 보내기
			else {
				if (GSessionManager.clients[pl]._obj_stat.race != RACE::RACE_PLAYER)	// player가 아니라면 패킷을 보낼 필요가 없다
					continue;

				GSessionManager.clients[pl]._ViewListLock.lock();
				if (0 == GSessionManager.clients[pl].view_list.count(c_id)) {
					GSessionManager.clients[pl].send_add_object(c_id);
					GSessionManager.clients[pl].view_list.insert(c_id);
					GSessionManager.clients[pl]._ViewListLock.unlock();
					GSessionManager.clients[pl]._s_state = ST_INGAME;
				}
				else {
					GSessionManager.clients[pl]._ViewListLock.unlock();
					GSessionManager.clients[pl].send_move_packet(c_id, 0);
				}
			}
		}

		for (auto pl : old_vl) {
			// old에는 있는데 new에는 없으면 삭제
			if (0 == new_vl.count(pl)) {
				GSessionManager.clients[c_id].Send_Remove_Packet(pl);
				GSessionManager.clients[c_id]._ViewListLock.lock();
				GSessionManager.clients[c_id].view_list.erase(pl);
				GSessionManager.clients[c_id]._ViewListLock.unlock();
				GSessionManager.clients[pl]._s_state = ST_SLEEP;

				if (GSessionManager.clients[pl]._obj_stat.race != RACE::RACE_PLAYER)	// player가 아니라면 패킷을 보낼 필요가 없다
					continue;
				GSessionManager.clients[pl]._ViewListLock.lock();
				if (0 == GSessionManager.clients[pl].view_list.count(c_id)) {
					GSessionManager.clients[pl]._ViewListLock.unlock();
				}
				else {
					GSessionManager.clients[pl].Send_Remove_Packet(c_id);
					GSessionManager.clients[pl].view_list.erase(c_id);
					GSessionManager.clients[pl]._ViewListLock.unlock();
					GSessionManager.clients[pl]._s_state = ST_SLEEP;
				}
			}
		}

		// Block 시야 처리
		// 부하가 많으면 나중에 섹터로 나눠야 함
		for (int i = 0; i < NUM_BLOCK; ++i) {
			if (GSessionManager.clients[c_id]._obj_stat.sector != GSessionManager.blocks[i].sector) continue;
			if (RANGE > GSessionManager.distance_block(c_id, i)) {
				GSessionManager.clients[c_id]._ViewListLock.lock();
				GSessionManager.clients[c_id].block_view_list.insert(i);
				GSessionManager.clients[c_id]._ViewListLock.unlock();
				GSessionManager.clients[c_id].send_add_block(i);
			}
			else {
				GSessionManager.clients[c_id]._ViewListLock.lock();
				if (0 != GSessionManager.clients[c_id].block_view_list.count(i)) {
					GSessionManager.clients[c_id].block_view_list.erase(i);
					GSessionManager.clients[c_id].Send_Remove_Block(i);
				}
				GSessionManager.clients[c_id]._ViewListLock.unlock();
			}
		}

		break;
	}
	case CS_ATTACK: {
		for (auto& obj : GSessionManager.clients[c_id].view_list) {
			if (GSessionManager.clients[obj]._s_state == ST_SLEEP) continue;					// 몬스터가 이미 죽었으면 공격 불가
			GSessionManager.clients[c_id].Send_PlayerAttack_Packet(c_id);
			if (GSessionManager.clients[obj]._obj_stat.race == RACE_PLAYER) {				// 플레이어끼리는 공격 불가
				GSessionManager.clients[c_id].Send_PlayerAttack_Packet(c_id);
				GSessionManager.clients[obj].Send_PlayerAttack_Packet(c_id);
				continue;
			}
			if (GSessionManager.distance(c_id, obj) < 2) {										// 공격 성공
				if (GSessionManager.clients[obj]._obj_stat.race == RACE_PLAYER) continue;				// 플레이어끼리는 공격 불가			
				GSessionManager.Hit_NPC(c_id, obj);

				for (auto& pl : GSessionManager.clients[c_id].view_list) {
					if (GSessionManager.clients[pl]._obj_stat.race != RACE_PLAYER) continue;
					GSessionManager.clients[pl].Send_StatChange_Packet(pl, obj);				// 공격한 플레이어 주변 플레이어들에게 결과 보냄
					GSessionManager.clients[pl].Send_StatChange_Packet(c_id, obj);				// 공격한 플레이어에게 전투 결과 보냄
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

		for (int& connected_id : GSessionManager.ConnectedPlayer)
			GSessionManager.clients[connected_id].do_send(&chat_packet);

		cout << "[" << GSessionManager.clients[c_id]._obj_stat._name << "] : " << p->mess << "\n";
		break;
	}
	case CS_PARTY_INVITE:
	{
		CS_PARTY_INVITE_PACKET* p = reinterpret_cast<CS_PARTY_INVITE_PACKET*>(packet);
		for (int& connected_id : GSessionManager.ConnectedPlayer) {
			if (GSessionManager.clients[connected_id]._obj_stat._id == p->master_id) continue;
			if (GSessionManager.distance(connected_id, p->master_id) < 2)
			{
				SC_PARTY_INVITE_PACKET party_packet;
				party_packet.size = sizeof(SC_PARTY_INVITE_PACKET);
				party_packet.type = SC_PARTY_INVITE;
				party_packet.id = p->master_id;

				GSessionManager.clients[connected_id].do_send(&party_packet);
			}

		}
		break;
	}
	case CS_PARTY:
	{
		CS_PARTY_PACKET* p = reinterpret_cast<CS_PARTY_PACKET*>(packet);
		if (true == p->allow) {
			GSessionManager.clients[p->master_id].my_party.push_back(p->id);
			GSessionManager.clients[p->id].my_party.push_back(p->master_id);

			SC_PARTY_PACKET send_player;
			send_player.type = SC_PARTY;
			send_player.size = sizeof(SC_PARTY_PACKET);
			send_player.id = p->master_id;

			GSessionManager.clients[p->id].do_send(&send_player);

			SC_PARTY_PACKET send_another_player;
			send_another_player.type = SC_PARTY;
			send_another_player.size = sizeof(SC_PARTY_PACKET);
			send_another_player.id = p->id;
			GSessionManager.clients[p->master_id].do_send(&send_another_player);
		}
		break;
	}
	}

}

void disconnect(int c_id)
{
	GSessionManager.clients[c_id]._lock.lock();
	if (GSessionManager.clients[c_id]._s_state == ST_FREE) {
		GSessionManager.clients[c_id]._lock.unlock();
		return;
	}
	closesocket(GSessionManager.clients[c_id]._socket);
	GSessionManager.clients[c_id]._s_state = ST_FREE;
	GSessionManager.clients[c_id]._lock.unlock();

	for (auto& pl : GSessionManager.clients) {
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
			int client_id = GSessionManager.get_new_client_id();
			if (client_id != -1) {
				GSessionManager.clients[client_id]._obj_stat.x = 0;
				GSessionManager.clients[client_id]._obj_stat.y = 0;
				GSessionManager.clients[client_id]._obj_stat._id = client_id;
				GSessionManager.clients[client_id]._obj_stat._name[0] = 0;
				GSessionManager.clients[client_id]._prev_remain = 0;
				GSessionManager.clients[client_id]._socket = c_socket;
				CreateIoCompletionPort(reinterpret_cast<HANDLE>(c_socket),
					g_h_iocp, client_id, 0);
				GSessionManager.clients[client_id].do_recv();
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
			int remain_data = num_bytes + GSessionManager.clients[key]._prev_remain;
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
			GSessionManager.clients[key]._prev_remain = remain_data;
			if (remain_data > 0) {
				memcpy(ex_over->_send_buf, p, remain_data);
			}
			GSessionManager.clients[key].do_recv();
			break;
		}
		case OP_SEND:
			if (0 == num_bytes) disconnect(client_id);
			delete ex_over;
			break;
		case OP_RANDOM_MOVE: {
			// 일단 그냥 ROMING
			int npc_id = static_cast<int>(key);
			GSessionManager.Move_NPC(npc_id, ex_over->target_id);
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

		for (int npc = MAX_USER; npc < MAX_USER + NUM_NPC; ++npc)
		{
			if (GSessionManager.clients[npc]._s_state == ST_SLEEP) continue;
			for (auto& c_id : GSessionManager.ConnectedPlayer)
			{
				if (GSessionManager.clients[c_id]._obj_stat.sector != GSessionManager.clients[npc]._obj_stat.sector) continue;

				if (GSessionManager.distance(npc, c_id) < RANGE)
				{
					auto ex_over = new OVER_EXP;
					ex_over->_comp_type = OP_RANDOM_MOVE;
					ex_over->target_id = c_id;
					PostQueuedCompletionStatus(g_h_iocp, 1, npc, &ex_over->_over);
				}
				else if (GSessionManager.distance(npc, c_id) < RANGE + 1)
				{
					GSessionManager.clients[c_id]._ViewListLock.lock();
					GSessionManager.clients[c_id].view_list.erase(npc);
					GSessionManager.clients[c_id]._ViewListLock.unlock();
					GSessionManager.clients[c_id].Send_Remove_Packet(npc);
				}
			}


		}
		auto end_t = chrono::system_clock::now();
		auto ai_t = end_t - start_t;
		this_thread::sleep_until(start_t + chrono::seconds(1));
	}

}

int main()
{
	GSessionManager.Init_Block();
	GSessionManager.Init_npc();
	GSessionManager.initialize_DB();

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
	a_over._wsabuf.buf = reinterpret_cast<CHAR*>(c_socket);
	AcceptEx(g_s_socket, c_socket, a_over._send_buf, 0, addr_size + 16, addr_size + 16, 0, &a_over._over);

	vector <thread> worker_threads;
	for (int i = 0; i < 9; ++i)
		worker_threads.emplace_back(do_worker);

	thread ai_thread{ do_ai_ver_heat_beat };
	ai_thread.join();

	for (auto& th : worker_threads)
		th.join();
	closesocket(g_s_socket);
	WSACleanup();
}
