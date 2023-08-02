#include "pch.h"
#include "Session.h"
#include "SessionManager.h"
#include "OVER_EXP.h"

void Session::do_recv()
{
	DWORD recv_flag = 0;
	memset(&_recv_over._over, 0, sizeof(_recv_over._over));
	_recv_over._wsabuf.len = BUF_SIZE - _prev_remain;
	_recv_over._wsabuf.buf = _recv_over._send_buf + _prev_remain;
	WSARecv(_socket, &_recv_over._wsabuf, 1, 0, &recv_flag,
		&_recv_over._over, 0);
}

void Session::do_send(void* packet)
{
	OVER_EXP* sdata = new OVER_EXP{ reinterpret_cast<char*>(packet) };
	WSASend(_socket, &sdata->_wsabuf, 1, 0, 0, &sdata->_over, 0);
}

void Session::send_login_ok_packet(int c_id)
{
	SC_LOGIN_OK_PACKET p;
	p.size = sizeof(SC_LOGIN_OK_PACKET);
	p.type = SC_LOGIN_OK;
	///////////////DB에 있다면 DB에서 꺼내올 것들 //////////////////
	p.id = GSessionManager.clients[c_id]._obj_stat._id;

	{
		/*p.x = _obj_stat.x;
		p.y = _obj_stat.y;*/
		p.x = 12;
		p.y = 15;
		p.level = _obj_stat.level;
		p.exp = _obj_stat.exp;
		p.hpmax = _obj_stat.hpmax;
		p.hp = _obj_stat.hp;
		p.race = RACE::RACE_PLAYER;
	}

	//if (isStressTest) {
		/*p.x = rand() % W_WIDTH;
		p.y = rand() % W_HEIGHT;

		p.level = 1;
		p.exp = 100;
		p.hpmax = 100;
		p.hp = 100;
		p.race = RACE::RACE_PLAYER;

		GSessionManager.clients[p.id]._obj_stat.x = p.x;
		GSessionManager.clients[p.id]._obj_stat.y = p.y;*/
	//}

	////////////////////////////////////////////////////////////////
	GSessionManager.clients[p.id]._obj_stat.x = p.x;
	GSessionManager.clients[p.id]._obj_stat.y = p.y;
	GSessionManager.SetSector(RACE_PLAYER, p.id);
	do_send(&p);
}

void Session::send_move_packet(int c_id, int client_time)
{
	SC_MOVE_OBJECT_PACKET p;
	p.id = c_id;
	p.size = sizeof(SC_MOVE_OBJECT_PACKET);
	p.type = SC_MOVE_OBJECT;
	p.x = GSessionManager.clients[c_id]._obj_stat.x;
	p.y = GSessionManager.clients[c_id]._obj_stat.y;
	p.hp = GSessionManager.clients[c_id]._obj_stat.hp;
	p.hpmax = GSessionManager.clients[c_id]._obj_stat.hpmax;
	p.client_time = client_time;
	do_send(&p);
}

void Session::send_add_object(int c_id)
{
	SC_ADD_OBJECT_PACKET p;
	p.id = c_id;
	p.size = sizeof(SC_ADD_OBJECT_PACKET);
	p.type = SC_ADD_OBJECT;
	p.x = GSessionManager.clients[c_id]._obj_stat.x;
	p.y = GSessionManager.clients[c_id]._obj_stat.y;
	p.race = GSessionManager.clients[c_id]._obj_stat.race;
	p.level = GSessionManager.clients[c_id]._obj_stat.level;
	p.hp = GSessionManager.clients[c_id]._obj_stat.hp;
	p.hpmax = GSessionManager.clients[c_id]._obj_stat.hpmax;
	strcpy_s(p.name, GSessionManager.clients[c_id]._obj_stat._name);
	do_send(&p);
}

void Session::send_add_block(int _id)
{
	SC_ADD_OBJECT_PACKET p;
	p.id = _id;
	p.size = sizeof(SC_ADD_OBJECT_PACKET);
	p.type = SC_ADD_OBJECT;
	p.x = GSessionManager.blocks[_id].x;
	p.y = GSessionManager.blocks[_id].y;
	p.race = RACE_BLOCK;
	do_send(&p);
}

void Session::send_chat_packet(int c_id, const char* mess)
{
	SC_CHAT_PACKET p;
	p.id = c_id;
	p.size = sizeof(SC_CHAT_PACKET) - sizeof(p.mess) + strlen(mess) + 1;
	p.type = SC_CHAT;
	strcpy_s(p.mess, mess);
	do_send(&p);
}

void Session::Send_Remove_Packet(int c_id)
{
	SC_REMOVE_OBJECT_PACKET p;
	p.id = c_id;
	p.size = sizeof(p);
	p.type = SC_REMOVE_OBJECT;
	p.race = GSessionManager.clients[c_id]._obj_stat.race;
	do_send(&p);
}

void Session::Send_Remove_Block(int _id)
{
	SC_REMOVE_OBJECT_PACKET p;
	p.id = _id;
	p.size = sizeof(p);
	p.type = SC_REMOVE_OBJECT;
	p.race = RACE_BLOCK;
	do_send(&p);
}

void Session::Send_StatChange_Packet(int c_id, int n_id)
{
	SC_STAT_CHANGE_PACKET packet;
	packet.size = sizeof(SC_STAT_CHANGE_PACKET);
	packet.type = SC_STAT_CHANGE;
	packet.hp = GSessionManager.clients[n_id]._obj_stat.hp;
	packet.hpmax = GSessionManager.clients[n_id]._obj_stat.hpmax;
	packet.id = n_id;
	packet.level = GSessionManager.clients[n_id]._obj_stat.level;

	GSessionManager.clients[c_id].do_send(&packet);
}

void Session::Send_PlayerAttack_Packet(int c_id)
{
	SC_PLAYER_ATTACK_PACKET packet;
	packet.size = sizeof(SC_PLAYER_ATTACK_PACKET);
	packet.type = SC_PLAYER_ATTACK;
	packet.id = c_id;

	do_send(&packet);
}

void Session::Send_Notice_Packet(char _message[BUF_SIZE])
{
	SC_CHAT_PACKET chat_packet;
	chat_packet.size = sizeof(chat_packet) - sizeof(chat_packet.mess) + strlen(_message) + 1;
	chat_packet.type = SC_CHAT;
	chat_packet.chat_type = CHATTYPE_NOTICE;
	chat_packet.id = 9999;
	strcpy_s(chat_packet.mess, _message);

	do_send(&chat_packet);
}