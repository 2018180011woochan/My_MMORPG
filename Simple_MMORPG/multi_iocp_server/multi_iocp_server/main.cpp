#include "pch.h"
#include "Include.h"
#include "SessionManager.h"
#include "OVER_EXP.h"
#include "SectorManager.h"

// 해야댈거
// 로그인 실패 만들기						O
// 죽었을때 시야처리 잘 안되는거 수정		O
// 파티 경험치 공유						O
// 플레이어 5초마다 HP회복
// 몬스터 사망후 30초후 부활
// 무찔렀을 경우 경험치 : 레벨 * 레벨 * 2	O
// 어그로, 로밍 2배						O
// 공격속도 1초에 한번
// 몬스터 잡았을때 경험치 메시지			O

HANDLE GIocpHandle;
SOCKET GSocket;

void Disconnect(int c_id);

void ProcessPacket(int c_id, char* packet)
{
	switch (packet[1]) {
	case CS_LOGIN: {
		CS_LOGIN_PACKET* p = reinterpret_cast<CS_LOGIN_PACKET*>(packet);

		if (p->name[0] == '\0')
			break;

		for (int i = 0; i < MAX_USER; ++i)
		{
			if (GSessionManager.clients[i]._ObjStat.DBID == p->db_id)
			{
				if (GSessionManager.clients[i]._SessionState == ST_INGAME)
				{
					// loginfail보내기
					SC_LOGIN_FAIL_PACKET login_fail_packet;
					login_fail_packet.size = sizeof(SC_LOGIN_FAIL_PACKET);
					login_fail_packet.type = SC_LOGIN_FAIL;
					login_fail_packet.reason = 1;
					GSessionManager.clients[c_id].DoSend(&login_fail_packet);
					return;
				}
			}
		}

		GSessionManager.IsAllowAccess(p->db_id, c_id);

		GSessionManager.clients[c_id]._Lock.lock();
		strcpy_s(GSessionManager.clients[c_id]._ObjStat.Name, p->name);
		GSessionManager.clients[c_id]._ObjStat.ID = c_id;
		GSessionManager.clients[c_id]._ObjStat.DBID = p->db_id;
		GSessionManager.clients[c_id]._ObjStat.Race = RACE::RACE_PLAYER;
		GSessionManager.clients[c_id].SendLoginOkPacket(c_id);
		GSessionManager.clients[c_id]._SessionState = ST_INGAME;
		GSessionManager.clients[c_id]._Lock.unlock();

		GSessionManager.ConnectedPlayer.push_back(c_id);

		for (int i = 0; i < MAX_USER; ++i) {
			auto& pl = GSessionManager.clients[i];
			pl._Lock.lock();
			if (pl._ObjStat.ID == c_id) {
				pl._Lock.unlock();
				continue;
			}
			if (ST_INGAME != pl._SessionState) {
				pl._Lock.unlock();
				continue;
			}

			// 나중엔 거리가 가까운애들만 추가
			if (RANGE >= GSessionManager.Distance(c_id, pl._ObjStat.ID)) {
				pl._ViewListLock.lock();
				pl._ViewList.insert(c_id);
				pl._ViewListLock.unlock();
				pl.SendAddObjectPacket(c_id);
			}
			pl._Lock.unlock();
		}

		for (auto& obj : GSessionManager.clients) {
			if (obj._ObjStat.ID == c_id) continue;
			//if (ST_INGAME != obj._s_state) continue;
			if (ST_FREE == obj._SessionState) continue;

			if (RANGE >= GSessionManager.Distance(obj._ObjStat.ID, c_id)) {
				GSessionManager.clients[c_id]._ViewListLock.lock();
				GSessionManager.clients[c_id]._ViewList.insert(obj._ObjStat.ID);
				GSessionManager.clients[c_id]._ViewListLock.unlock();
				GSessionManager.clients[c_id].SendAddObjectPacket(obj._ObjStat.ID);
				obj._SessionState = ST_INGAME;
			}
		}

		for (auto& block : GSessionManager.blocks) {
			if (RANGE > GSessionManager.DistanceBlock(c_id, block.BlockID))
				GSessionManager.clients[c_id].SendAddBlockPacket(block.BlockID);

		}
		break;
	}
	case CS_MOVE: {
		// TODO 시야처리
		// LOCK FREE
		CS_MOVE_PACKET* p = reinterpret_cast<CS_MOVE_PACKET*>(packet);
		short x = GSessionManager.clients[c_id]._ObjStat.x;
		short y = GSessionManager.clients[c_id]._ObjStat.y;

		switch (p->direction) {
		case 0:
			if (GSessionManager.IsMovePossible(c_id, DIRECTION_UP) && y > 0) y--; break;
		case 1:
			if (GSessionManager.IsMovePossible(c_id, DIRECTION_DOWN) && y < W_HEIGHT - 1) y++; break;
		case 2:
			if (GSessionManager.IsMovePossible(c_id, DIRECTION_LEFT) && x > 0) x--; break;
		case 3:
			if (GSessionManager.IsMovePossible(c_id, DIRECTION_RIGHT) && x < W_WIDTH - 1) x++; break;
		}

		GSessionManager.clients[c_id]._ObjStat.x = x;
		GSessionManager.clients[c_id]._ObjStat.y = y;

		for (int i = 0; i < MAX_USER + NUM_NPC; ++i) {
			//if (GSessionManager.clients[i]._ObjStat.Sector != GSessionManager.clients[c_id]._ObjStat.Sector) continue;
			if (c_id == GSessionManager.clients[i]._ObjStat.ID) continue;
			if (!GSectorManager.isInSector(c_id, i)) continue;

			if (RANGE > GSessionManager.Distance(c_id, i)) {
				if (GSessionManager.clients[i]._ObjStat.Race == RACE_END ||
					GSessionManager.clients[i]._ObjStat.Race == RACE_BLOCK ||
					GSessionManager.clients[i]._ObjStat.IsDead) continue;
				GSessionManager.clients[i]._SessionState = ST_INGAME;
			}
		}
		///////////////////////////////////////////

		GSectorManager.PushSector(c_id);
		//GSessionManager.clients[c_id].SendMovePacket(c_id, 0);

//#pragma region 섹터 나누기전
//
//		GSessionManager.SetSector(GSessionManager.clients[c_id]._ObjStat.Race, c_id);
//
//		GSessionManager.clients[c_id]._ViewListLock.lock();
//		unordered_set<int> old_vl = GSessionManager.clients[c_id]._ViewList;
//		GSessionManager.clients[c_id]._ViewListLock.unlock();
//
//		unordered_set<int> new_vl;
//		for (int i = 0; i < MAX_USER + NUM_NPC; ++i) {
//			if (GSessionManager.clients[i]._ObjStat.Sector !=
//				GSessionManager.clients[c_id]._ObjStat.Sector) continue;
//
//			if (ST_INGAME != GSessionManager.clients[i]._SessionState) continue;
//			if (c_id == GSessionManager.clients[i]._ObjStat.ID) continue;
//
//			if (RANGE > GSessionManager.Distance(c_id, i))
//				new_vl.insert(i);
//		}
//		GSessionManager.clients[c_id].SendMovePacket(c_id, 0);
//
//		for (auto pl : new_vl) {
//			// old_vl에 없는데 new_vl에 있으면 add패킷 보내기
//			if (0 == old_vl.count(pl)) {
//				GSessionManager.clients[c_id].SendAddObjectPacket(pl);
//				GSessionManager.clients[c_id]._ViewListLock.lock();
//				GSessionManager.clients[c_id]._ViewList.insert(pl);
//				GSessionManager.clients[c_id]._ViewListLock.unlock();
//				GSessionManager.clients[pl]._SessionState = ST_INGAME;
//
//				if (GSessionManager.clients[pl]._ObjStat.Race != RACE::RACE_PLAYER)	// player가 아니라면 패킷을 보낼 필요가 없다
//					continue;
//				GSessionManager.clients[pl]._ViewListLock.lock();
//				if (0 == GSessionManager.clients[pl]._ViewList.count(c_id)) {
//					GSessionManager.clients[pl].SendAddObjectPacket(c_id);
//					GSessionManager.clients[pl]._ViewList.insert(c_id);
//					GSessionManager.clients[pl]._ViewListLock.unlock();
//					GSessionManager.clients[pl]._SessionState = ST_INGAME;
//				}
//				else {
//					GSessionManager.clients[pl]._ViewListLock.unlock();
//					GSessionManager.clients[pl].SendMovePacket(c_id, 0);
//				}
//			}
//			// old_vl에도 있고 new_vl에도 있으면 move패킷 보내기
//			else {
//				if (GSessionManager.clients[pl]._ObjStat.Race != RACE::RACE_PLAYER)	// player가 아니라면 패킷을 보낼 필요가 없다
//					continue;
//
//				GSessionManager.clients[pl]._ViewListLock.lock();
//				if (0 == GSessionManager.clients[pl]._ViewList.count(c_id)) {
//					GSessionManager.clients[pl].SendAddObjectPacket(c_id);
//					GSessionManager.clients[pl]._ViewList.insert(c_id);
//					GSessionManager.clients[pl]._ViewListLock.unlock();
//					GSessionManager.clients[pl]._SessionState = ST_INGAME;
//				}
//				else {
//					GSessionManager.clients[pl]._ViewListLock.unlock();
//					GSessionManager.clients[pl].SendMovePacket(c_id, 0);
//				}
//			}
//		}
//
//		for (auto pl : old_vl) {
//			// old에는 있는데 new에는 없으면 삭제
//			if (0 == new_vl.count(pl)) {
//				GSessionManager.clients[c_id].SendRemovePacket(pl);
//				GSessionManager.clients[c_id]._ViewListLock.lock();
//				GSessionManager.clients[c_id]._ViewList.erase(pl);
//				GSessionManager.clients[c_id]._ViewListLock.unlock();
//				GSessionManager.clients[pl]._SessionState = ST_SLEEP;
//
//				if (GSessionManager.clients[pl]._ObjStat.Race != RACE::RACE_PLAYER)	// player가 아니라면 패킷을 보낼 필요가 없다
//					continue;
//				GSessionManager.clients[pl]._ViewListLock.lock();
//				if (0 == GSessionManager.clients[pl]._ViewList.count(c_id)) {
//					GSessionManager.clients[pl]._ViewListLock.unlock();
//				}
//				else {
//					GSessionManager.clients[pl].SendRemovePacket(c_id);
//					GSessionManager.clients[pl]._ViewList.erase(c_id);
//					GSessionManager.clients[pl]._ViewListLock.unlock();
//					GSessionManager.clients[pl]._SessionState = ST_SLEEP;
//				}
//			}
//		}
//
//#pragma endregion 섹터 처리 후

		// Block 시야 처리
		// 부하가 많으면 나중에 섹터로 나눠야 함
		for (int i = 0; i < NUM_BLOCK; ++i) {
			//if (GSessionManager.clients[c_id]._ObjStat.Sector != GSessionManager.blocks[i].Sector) continue;
			if (RANGE > GSessionManager.DistanceBlock(c_id, i)) {
				GSessionManager.clients[c_id]._ViewListLock.lock();
				GSessionManager.clients[c_id]._ViewListBlock.insert(i);
				GSessionManager.clients[c_id]._ViewListLock.unlock();
				GSessionManager.clients[c_id].SendAddBlockPacket(i);
			}
			else {
				GSessionManager.clients[c_id]._ViewListLock.lock();
				if (0 != GSessionManager.clients[c_id]._ViewListBlock.count(i)) {
					GSessionManager.clients[c_id]._ViewListBlock.erase(i);
					GSessionManager.clients[c_id].SendRemoveBlockPacket(i);
				}
				GSessionManager.clients[c_id]._ViewListLock.unlock();
			}
		}

		break;
	}
	case CS_ATTACK: {
		for (auto& obj : GSessionManager.clients[c_id]._ViewList) {
			if (GSessionManager.clients[obj]._SessionState == ST_SLEEP) continue;					// 몬스터가 이미 죽었으면 공격 불가
			GSessionManager.clients[c_id].SendPlayerAttackPacket(c_id);
			if (GSessionManager.clients[obj]._ObjStat.Race == RACE_PLAYER) {				// 플레이어끼리는 공격 불가
				GSessionManager.clients[c_id].SendPlayerAttackPacket(c_id);
				GSessionManager.clients[obj].SendPlayerAttackPacket(c_id);
				continue;
			}
			if (GSessionManager.Distance(c_id, obj) < 2) {										// 공격 성공
				if (GSessionManager.clients[obj]._ObjStat.Race == RACE_PLAYER) continue;				// 플레이어끼리는 공격 불가			
				GSessionManager.HitNPC(c_id, obj);

				for (auto& pl : GSessionManager.clients[c_id]._ViewList) {
					if (GSessionManager.clients[pl]._ObjStat.Race != RACE_PLAYER) continue;
					GSessionManager.clients[pl].SendStatChangePacket(pl, obj);				// 공격한 플레이어 주변 플레이어들에게 결과 보냄
					GSessionManager.clients[pl].SendStatChangePacket(c_id, obj);				// 공격한 플레이어에게 전투 결과 보냄
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
			GSessionManager.clients[connected_id].DoSend(&chat_packet);

		cout << "[" << GSessionManager.clients[c_id]._ObjStat.Name << "]:" << p->mess << "\n";
		break;
	}
	case CS_PARTY_INVITE:
	{
		CS_PARTY_INVITE_PACKET* p = reinterpret_cast<CS_PARTY_INVITE_PACKET*>(packet);
		for (int& connected_id : GSessionManager.ConnectedPlayer) {
			if (GSessionManager.clients[connected_id]._ObjStat.ID == p->master_id) continue;
			if (GSessionManager.Distance(connected_id, p->master_id) < 2)
			{
				SC_PARTY_INVITE_PACKET party_packet;
				party_packet.size = sizeof(SC_PARTY_INVITE_PACKET);
				party_packet.type = SC_PARTY_INVITE;
				party_packet.id = p->master_id;

				GSessionManager.clients[connected_id].DoSend(&party_packet);
			}

		}
		break;
	}
	case CS_PARTY:
	{
		CS_PARTY_PACKET* p = reinterpret_cast<CS_PARTY_PACKET*>(packet);
		if (true == p->allow) {
			//GSessionManager.clients[p->master_id]._MyParty.push_back(p->id);
			GSessionManager.clients[p->master_id]._MyParty.emplace(p->id);
			//GSessionManager.clients[p->id]._MyParty.push_back(p->master_id);
			GSessionManager.clients[p->id]._MyParty.emplace(p->master_id);

			SC_PARTY_PACKET send_player;
			send_player.type = SC_PARTY;
			send_player.size = sizeof(SC_PARTY_PACKET);
			send_player.id = p->master_id;

			GSessionManager.clients[p->id].DoSend(&send_player);

			SC_PARTY_PACKET send_another_player;
			send_another_player.type = SC_PARTY;
			send_another_player.size = sizeof(SC_PARTY_PACKET);
			send_another_player.id = p->id;
			GSessionManager.clients[p->master_id].DoSend(&send_another_player);
		}
		break;
	}
	}

}

void Disconnect(int c_id)
{
	GSessionManager.clients[c_id]._Lock.lock();
	if (GSessionManager.clients[c_id]._SessionState == ST_FREE) {
		GSessionManager.clients[c_id]._Lock.unlock();
		return;
	}
	closesocket(GSessionManager.clients[c_id]._Socket);
	GSessionManager.clients[c_id]._SessionState = ST_FREE;
	GSessionManager.clients[c_id]._Lock.unlock();

	for (auto& pl : GSessionManager.clients) {
		if (pl._ObjStat.ID == c_id) continue;
		pl._Lock.lock();
		if (pl._SessionState != ST_INGAME) {
			pl._Lock.unlock();
			continue;
		}
		pl.SendRemovePacket(c_id);
		pl._Lock.unlock();
	}

}

void Dispatch()
{
	while (true) {
		DWORD num_bytes;
		ULONG_PTR key;
		WSAOVERLAPPED* over = nullptr;
		BOOL ret = GetQueuedCompletionStatus(GIocpHandle, &num_bytes, &key, &over, INFINITE);
		OVER_EXP* ex_over = reinterpret_cast<OVER_EXP*>(over);
		int client_id = static_cast<int>(key);
		if (FALSE == ret) {
			if (ex_over->_CompType == OP_ACCEPT) cout << "Accept Error";
			else {
				cout << "GQCS Error on client[" << key << "]\n";
				Disconnect(static_cast<int>(key));
				if (ex_over->_CompType == OP_SEND) delete ex_over;
				continue;
			}
		}

		switch (ex_over->_CompType) {
		case OP_ACCEPT: {
			SOCKET c_socket = reinterpret_cast<SOCKET>(ex_over->_WsaBuf.buf);
			int client_id = GSessionManager.GetNewClientID();
			if (client_id != -1) {
				GSessionManager.clients[client_id]._ObjStat.x = 0;
				GSessionManager.clients[client_id]._ObjStat.y = 0;
				GSessionManager.clients[client_id]._ObjStat.ID = client_id;
				GSessionManager.clients[client_id]._ObjStat.Name[0] = 0;
				GSessionManager.clients[client_id]._PrevRemainTime = 0;
				GSessionManager.clients[client_id]._Socket = c_socket;
				CreateIoCompletionPort(reinterpret_cast<HANDLE>(c_socket),
					GIocpHandle, client_id, 0);
				GSessionManager.clients[client_id].DoRecv();
				c_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
			}
			else {
				cout << "Max user exceeded.\n";
			}
			ZeroMemory(&ex_over->_Over, sizeof(ex_over->_Over));
			ex_over->_WsaBuf.buf = reinterpret_cast<CHAR*>(c_socket);
			int addr_size = sizeof(SOCKADDR_IN);
			AcceptEx(GSocket, c_socket, ex_over->_SendBuf, 0, addr_size + 16, addr_size + 16, 0, &ex_over->_Over);
			break;
		}
		case OP_RECV: {
			if (0 == num_bytes) Disconnect(client_id);
			int remain_data = num_bytes + GSessionManager.clients[key]._PrevRemainTime;
			char* p = ex_over->_SendBuf;
			while (remain_data > 0) {
				int packet_size = p[0];
				if (packet_size <= remain_data) {
					ProcessPacket(static_cast<int>(key), p);
					p = p + packet_size;
					remain_data = remain_data - packet_size;
				}
				else break;
			}
			GSessionManager.clients[key]._PrevRemainTime = remain_data;
			if (remain_data > 0) {
				memcpy(ex_over->_SendBuf, p, remain_data);
			}
			GSessionManager.clients[key].DoRecv();
			break;
		}
		case OP_SEND:
			if (0 == num_bytes) Disconnect(client_id);
			delete ex_over;
			break;
		case OP_RANDOM_MOVE: {
			// 일단 그냥 ROMING
			int npc_id = static_cast<int>(key);
			// 이거 땜에 테스트할때 막 퍼짐
			GSessionManager.MoveNPC(npc_id, ex_over->_TargetID);
			delete ex_over;
		}
						   break;
		} //SWITCH
	} // WHILE
}

void DoAiHeatBeat()
{
	for (;;) {
		auto start_t = chrono::system_clock::now();

		for (int npc = MAX_USER; npc < MAX_USER + NUM_NPC; ++npc)
		{
			if (GSessionManager.clients[npc]._SessionState == ST_SLEEP) continue;
			for (auto& c_id : GSessionManager.ConnectedPlayer)
			{
				//if (GSessionManager.clients[c_id]._ObjStat.Sector != 
				//	GSessionManager.clients[npc]._ObjStat.Sector) continue;

				if (GSessionManager.Distance(npc, c_id) < RANGE)
				{
					auto ex_over = new OVER_EXP;
					ex_over->_CompType = OP_RANDOM_MOVE;
					ex_over->_TargetID = c_id;
					PostQueuedCompletionStatus(GIocpHandle, 1, npc, &ex_over->_Over);
				}
				else if (GSessionManager.Distance(npc, c_id) < RANGE + 1)
				{
					GSessionManager.clients[c_id]._ViewListLock.lock();
					GSessionManager.clients[c_id]._ViewList.erase(npc);
					GSessionManager.clients[c_id]._ViewListLock.unlock();
					GSessionManager.clients[c_id].SendRemovePacket(npc);
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
	GSectorManager.Init();
	GSessionManager.InitBlock();
	GSessionManager.InitNpc();
	GSessionManager.InitDB();


	WSADATA WSAData;
	WSAStartup(MAKEWORD(2, 2), &WSAData);
	GSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	SOCKADDR_IN ServerAddr;
	memset(&ServerAddr, 0, sizeof(ServerAddr));
	ServerAddr.sin_family = AF_INET;
	ServerAddr.sin_port = htons(PORT_NUM);
	ServerAddr.sin_addr.S_un.S_addr = INADDR_ANY;
	bind(GSocket, reinterpret_cast<sockaddr*>(&ServerAddr), sizeof(ServerAddr));
	listen(GSocket, SOMAXCONN);
	SOCKADDR_IN ClientAddr;
	int AddrSize = sizeof(ClientAddr);
	int ClientID = 0;

	GIocpHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
	CreateIoCompletionPort(reinterpret_cast<HANDLE>(GSocket), GIocpHandle, 9999, 0);
	SOCKET c_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	OVER_EXP a_over;
	a_over._CompType = OP_ACCEPT;
	a_over._WsaBuf.buf = reinterpret_cast<CHAR*>(c_socket);
	AcceptEx(GSocket, c_socket, a_over._SendBuf, 0, AddrSize + 16, AddrSize + 16, 0, &a_over._Over);

	vector <thread> worker_threads;
	for (int i = 0; i < 9; ++i)
		worker_threads.emplace_back(Dispatch);

	thread ai_thread{ DoAiHeatBeat };
	ai_thread.join();

	for (auto& th : worker_threads)
		th.join();
	closesocket(GSocket);
	WSACleanup();
}
