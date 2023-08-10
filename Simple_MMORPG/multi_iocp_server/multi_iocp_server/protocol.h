#pragma once
#ifndef __PROTOCOL__H_
#define __PROTOCOL__H_


constexpr int PORT_NUM = 4000;
constexpr int BUF_SIZE = 200;
constexpr int NAME_SIZE = 20;

constexpr int W_WIDTH = 2000;
constexpr int W_HEIGHT = 2000;

constexpr int Half_width = W_WIDTH / 2;
constexpr int Half_height = W_HEIGHT / 2;

constexpr int MAX_USER = 10000;
constexpr int NUM_NPC = 200000;
constexpr int NUM_BLOCK = 80000;



// Packet ID
constexpr char CS_LOGIN = 0;
constexpr char CS_MOVE = 1;
constexpr char CS_ATTACK = 2;
constexpr char CS_CHAT = 3;
constexpr char CS_PARTY_INVITE = 4;
constexpr char CS_PARTY = 5;

constexpr char SC_LOGIN_OK = 11;
constexpr char SC_LOGIN_FAIL = 12;
constexpr char SC_ADD_OBJECT = 13;
constexpr char SC_REMOVE_OBJECT = 14;
constexpr char SC_MOVE_OBJECT = 15;
constexpr char SC_CHAT = 16;
constexpr char SC_STAT_CHANGE = 17;
constexpr char SC_PLAYER_ATTACK = 18;
constexpr char SC_PARTY_INVITE = 19;
constexpr char SC_PARTY = 20;

#pragma pack (push, 1)
struct CS_LOGIN_PACKET {
	unsigned char size;
	char	type;
	char	name[NAME_SIZE];
	int		db_id;
};

struct CS_MOVE_PACKET {
	unsigned char size;
	char	type;
	char	direction;  // 0 : UP, 1 : DOWN, 2 : LEFT, 3 : RIGHT
	unsigned  client_time;
};

struct CS_ATTACK_PACKET {
	unsigned char size;
	char	type;
};

struct CS_CHAT_PACKET {
	unsigned char size;
	char	type;
	int		target_id;
	char	chat_type;			// 1 : say,  2 : tell, 3 : shout
	char	mess[BUF_SIZE];
};

/////////////////////////////////////////////////// 
struct CS_PARTY_INVITE_PACKET {
	unsigned char size;
	char	type;
	int		master_id;
};

struct CS_PARTY_PACKET {
	unsigned char size;
	char	type;
	int		master_id;
	int		id;
	bool	allow;
};
///////////////////////////////////////////////////

struct SC_LOGIN_OK_PACKET {
	unsigned char size;
	char	type;
	int	id;
	short race;
	short x, y;
	short level;
	int	  exp;
	int   hp, hpmax;
	short   sector;
};

struct SC_LOGIN_FAIL_PACKET {
	unsigned char size;
	char	type;
	int		reason;				// 0 : Invalid Name  (특수문자, 공백 제외)
								// 1 : Name Already Playing
								// 2 : Server Full
};

struct SC_ADD_OBJECT_PACKET {
	unsigned char size;
	char	type;			
	int		id;
	short	x, y;
	short	race;			// 종족 : 인간, 엘프, 드워프, 오크, 드래곤
							// 클라이언트에서 종족별로 별도의 그래픽 표현
							// 추가적으로 성별이나, 직업을 추가할 수 있다.
	char	name[NAME_SIZE];
	short	level;
	int		hp, hpmax;
	short   sector;
};

struct SC_REMOVE_OBJECT_PACKET {
	unsigned char size;
	char	type;
	int	id;
	short race;
};

struct SC_MOVE_OBJECT_PACKET {
	unsigned char size;
	char	type;
	int	id;
	short	x, y;
	int		hp, hpmax;
	short	sector;
	unsigned int client_time;
};

struct SC_CHAT_PACKET {
	unsigned char size;
	char	type;
	int		id;				// 0 - 100만 : Object ID
							// -1 : System Message
	char	chat_type;		// 0 : 일반 채팅 (say)
							// 1 : 1:1 통신 (tell)
							// 2 : 방송 (shout)
							// 3 : 공지
	char	mess[BUF_SIZE];
};

struct SC_STAT_CHANGE_PACKET {
	unsigned char size;
	char	type;
	int		id;				
	short	level;
	int		exp;
	int		hp, hpmax;
};

/////////////////////////////////////////////////// 
struct SC_PLAYER_ATTACK_PACKET {	// 구현중...
	unsigned char size;
	char	type;
	int		id;
};

struct SC_PARTY_INVITE_PACKET {
	unsigned char size;
	char	type;
	int		id;
};

struct SC_PARTY_PACKET {
	unsigned char size;
	char	type;
	int		id;
};
///////////////////////////////////////////////////

#pragma pack (pop)


#endif // !__PROTOCOL__H_
