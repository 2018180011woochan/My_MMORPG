#include "pch.h"
#include "../../Server/protocol.h"
#include "ClientPacketHandler.h"
#include "BufferReader.h"


void ClientPacketHandler::HandlePacket(BYTE* buffer, int32 len)
{
	BufferReader br(buffer, len);

	PacketHeader header;
	br >> header;

	switch (header.id)
	{
	case SC_TEST_OK:
	{
		Handle_SC_TEST_OK(buffer, len);	
		break;
	}
	default:
		cout << "error~!!" << endl;
		//printf("Unknown PACKET type [%d]\n", ptr[1]);
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

	wstring name;

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

	wstring name;
	uint16 nameLen;
	br >> nameLen;
	name.resize(nameLen);

	br.Read((void*)name.data(), nameLen * sizeof(WCHAR));

	wcout.imbue(std::locale("kor"));
	wcout << name << endl;
}

void ClientPacketHandler::Handle_SC_TEST_OK(BYTE* buffer, int32 len)
{
	BufferReader br(buffer, len);

	PacketHeader header;
	br >> header;

	bool isok;
	uint64 id;

	br >> isok >> id ;

	cout << "isok : " << isok << " ID : " << id << endl;

	wstring name;
	uint16 nameLen;
	br >> nameLen;
	name.resize(nameLen);

	br.Read((void*)name.data(), nameLen * sizeof(WCHAR));

	wcout.imbue(std::locale("kor"));
	wcout << name << endl;
}
