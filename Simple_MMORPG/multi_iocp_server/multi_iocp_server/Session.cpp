#include "pch.h"
#include "Session.h"
#include "SessionManager.h"
#include "OVER_EXP.h"

void Session::DoRecv()
{
	DWORD recv_flag = 0;
	memset(&_RecvOver._Over, 0, sizeof(_RecvOver._Over));
	_RecvOver._WsaBuf.len = BUF_SIZE - _PrevRemainTime;
	_RecvOver._WsaBuf.buf = _RecvOver._SendBuf + _PrevRemainTime;
	WSARecv(_Socket, &_RecvOver._WsaBuf, 1, 0, &recv_flag,
		&_RecvOver._Over, 0);
}

void Session::DoSend(void* packet)
{
	OVER_EXP* sdata = new OVER_EXP{ reinterpret_cast<char*>(packet) };
	WSASend(_Socket, &sdata->_WsaBuf, 1, 0, 0, &sdata->_Over, 0);
}

void Session::SendLoginOkPacket(int c_id)
{
	SC_LOGIN_OK_PACKET p;
	p.size = sizeof(SC_LOGIN_OK_PACKET);
	p.type = SC_LOGIN_OK;
	///////////////DB에 있다면 DB에서 꺼내올 것들 //////////////////
	p.id = GSessionManager.clients[c_id]._ObjStat.ID;

	{
		//p.x = _ObjStat.x;
		//p.y = _ObjStat.y;
		p.x = 12;
		p.y = 15;
		p.level = _ObjStat.Level;
		p.exp = _ObjStat.Exp;
		p.hpmax = _ObjStat.MaxHP;
		p.hp = _ObjStat.HP;
		p.race = RACE::RACE_PLAYER;
		p.sector = _ObjStat.SectorID;
	}

	//if (isStressTest) {
		/*p.x = rand() % W_WIDTH;
		p.y = rand() % W_HEIGHT;

		p.level = 1;
		p.exp = 100;
		p.hpmax = 100;
		p.hp = 100;
		p.race = RACE::RACE_PLAYER;
		p.sector = _ObjStat.SectorID;

		GSessionManager.clients[p.id]._ObjStat.x = p.x;
		GSessionManager.clients[p.id]._ObjStat.y = p.y;*/
	//}

	////////////////////////////////////////////////////////////////
	GSessionManager.clients[p.id]._ObjStat.x = p.x;
	GSessionManager.clients[p.id]._ObjStat.y = p.y;
	DoSend(&p);
}

void Session::SendMovePacket(int c_id, int client_time)
{
	SC_MOVE_OBJECT_PACKET p;
	p.id = c_id;
	p.size = sizeof(SC_MOVE_OBJECT_PACKET);
	p.type = SC_MOVE_OBJECT;
	p.x = GSessionManager.clients[c_id]._ObjStat.x;
	p.y = GSessionManager.clients[c_id]._ObjStat.y;
	p.hp = GSessionManager.clients[c_id]._ObjStat.HP;
	p.hpmax = GSessionManager.clients[c_id]._ObjStat.MaxHP;
	p.client_time = client_time;
	p.sector = GSessionManager.clients[c_id]._ObjStat.SectorID;
	DoSend(&p);
}

void Session::SendAddObjectPacket(int c_id)
{
	SC_ADD_OBJECT_PACKET p;
	p.id = c_id;
	p.size = sizeof(SC_ADD_OBJECT_PACKET);
	p.type = SC_ADD_OBJECT;
	p.x = GSessionManager.clients[c_id]._ObjStat.x;
	p.y = GSessionManager.clients[c_id]._ObjStat.y;
	p.race = GSessionManager.clients[c_id]._ObjStat.Race;
	p.level = GSessionManager.clients[c_id]._ObjStat.Level;
	p.hp = GSessionManager.clients[c_id]._ObjStat.HP;
	p.hpmax = GSessionManager.clients[c_id]._ObjStat.MaxHP;
	strcpy_s(p.name, GSessionManager.clients[c_id]._ObjStat.Name);
	DoSend(&p);
}

void Session::SendAddBlockPacket(int _id)
{
	SC_ADD_OBJECT_PACKET p;
	p.id = _id;
	p.size = sizeof(SC_ADD_OBJECT_PACKET);
	p.type = SC_ADD_OBJECT;
	p.x = GSessionManager.blocks[_id].x;
	p.y = GSessionManager.blocks[_id].y;
	p.race = RACE_BLOCK;
	DoSend(&p);
}

void Session::SendChatPacket(int c_id, const char* mess)
{
	SC_CHAT_PACKET p;
	p.id = c_id;
	p.size = sizeof(SC_CHAT_PACKET) - sizeof(p.mess) + strlen(mess) + 1;
	p.type = SC_CHAT;
	strcpy_s(p.mess, mess);
	DoSend(&p);
}

void Session::SendRemovePacket(int c_id)
{
	SC_REMOVE_OBJECT_PACKET p;
	p.id = c_id;
	p.size = sizeof(p);
	p.type = SC_REMOVE_OBJECT;
	p.race = GSessionManager.clients[c_id]._ObjStat.Race;
	DoSend(&p);
}

void Session::SendRemoveBlockPacket(int _id)
{
	SC_REMOVE_OBJECT_PACKET p;
	p.id = _id;
	p.size = sizeof(p);
	p.type = SC_REMOVE_OBJECT;
	p.race = RACE_BLOCK;
	DoSend(&p);
}

void Session::SendStatChangePacket(int c_id, int n_id)
{
	SC_STAT_CHANGE_PACKET packet;
	packet.size = sizeof(SC_STAT_CHANGE_PACKET);
	packet.type = SC_STAT_CHANGE;
	packet.hp = GSessionManager.clients[n_id]._ObjStat.HP;
	packet.hpmax = GSessionManager.clients[n_id]._ObjStat.MaxHP;
	packet.id = n_id;
	packet.level = GSessionManager.clients[n_id]._ObjStat.Level;

	GSessionManager.clients[c_id].DoSend(&packet);
}

void Session::SendPlayerAttackPacket(int c_id)
{
	SC_PLAYER_ATTACK_PACKET packet;
	packet.size = sizeof(SC_PLAYER_ATTACK_PACKET);
	packet.type = SC_PLAYER_ATTACK;
	packet.id = c_id;

	DoSend(&packet);
}

void Session::SendNoticePacket(char _message[BUF_SIZE])
{
	SC_CHAT_PACKET chat_packet;
	chat_packet.size = sizeof(chat_packet) - sizeof(chat_packet.mess) + strlen(_message) + 1;
	chat_packet.type = SC_CHAT;
	chat_packet.chat_type = CHATTYPE_NOTICE;
	chat_packet.id = 9999;
	strcpy_s(chat_packet.mess, _message);

	DoSend(&chat_packet);
}