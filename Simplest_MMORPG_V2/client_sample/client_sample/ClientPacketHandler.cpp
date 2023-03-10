#include "pch.h"
#include "../../Server/protocol.h"
#include "ClientPacketHandler.h"
#include "BufferReader.h"
#include "BufferWriter.h"


void ClientPacketHandler::HandlePacket(BYTE* buffer, int32 len)
{
	BufferReader br(buffer, len);

	PacketHeader header;
	br >> header;

	switch (header.id)
	{
	case SC_LOGIN:
	{
		Handle_SC_LOGIN(buffer, len);	
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

SendBufferRef ClientPacketHandler::Make_CS_LOGIN(wstring name)
{
	SendBufferRef sendBuffer = make_shared<SendBuffer>(4096);

	BufferWriter bw(sendBuffer->Buffer(), 4096);

	PacketHeader* header = bw.Reserve<PacketHeader>();

	// 가변 데이터
	bw << (uint16)name.size();
	bw.Write((void*)name.data(), name.size() * sizeof(WCHAR));

	header->size = bw.WriteSize();
	header->id = CS_LOGIN;	// 1 : hello message

	sendBuffer->CopyData(bw.GetBuffer(), bw.WriteSize());
	//Send(sendBuffer);

	return sendBuffer;
}

void ClientPacketHandler::Handle_SC_LOGIN(BYTE* buffer, int32 len)
{
	BufferReader br(buffer, len);

	PacketHeader header;
	br >> header;

	uint64 id;

	br >> id ;

	cout  << " ID : " << id << endl;

	wstring name;
	uint16 nameLen;
	br >> nameLen;
	name.resize(nameLen);

	br.Read((void*)name.data(), nameLen * sizeof(WCHAR));

	wcout.imbue(std::locale("kor"));
	wcout << name << endl;
}
