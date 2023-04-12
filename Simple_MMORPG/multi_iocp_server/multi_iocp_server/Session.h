#pragma once
#ifndef __SESSION_H_
#define __SESSION_H_

#include "Include.h"

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

class Session {
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
	//chrono::system_clock::time_point next_move_time;
	int		_prev_remain;

public:
	Session()
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

	~Session() {}

	void do_recv();


	void do_send(void* packet);


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


#endif // !__SESSION_H_
