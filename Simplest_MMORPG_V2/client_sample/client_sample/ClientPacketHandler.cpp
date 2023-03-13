#include "pch.h"
#include "ClientPacketHandler.h"
#include "BufferReader.h"

void ClientPacketHandler::HandlePacket(BYTE* buffer, int32 len)
{
	BufferReader br(buffer, len);

	PacketHeader header;
	br >> header;

	switch (header.id)
	{
	case S_TEST:
		Handle_S_TEST(buffer, len);
		break;

	default:
		break;
	}

	
}

// 패킷설계 temp
struct BuffData
{
	uint64 buffid;
	float femainTime;
};

struct S_TEXT
{
	uint64 id;
	uint32 hp;
	uint16 attack;
	// 가변 데이터
	vector<int64> buffs;
};

void ClientPacketHandler::Handle_S_TEST(BYTE* buffer, int32 len)
{
	BufferReader br(buffer, len);

	PacketHeader header;
	br >> header;

	uint64 id;
	uint32 hp;
	uint16 attack;
	br >> id >> hp >> attack;

	cout << "ID : " << id << " HP : " << hp << " ATTACK: " << attack << endl;

	vector<BuffData> buffs;
	uint16 buffCnt;
	br >> buffCnt;

	buffs.resize(buffCnt);

	for (int32 i = 0; i < buffCnt; ++i)
		br >> buffs[i].buffid >> buffs[i].femainTime;

	cout << "BuffCount : " << buffCnt << endl;

	for (int32 i = 0; i < buffCnt; ++i)
		cout << "BuffInfo : " << buffs[i].buffid << " " << buffs[i].femainTime << endl;
}
