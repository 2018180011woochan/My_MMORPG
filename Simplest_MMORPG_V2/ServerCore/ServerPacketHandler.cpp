#include "pch.h"
#include "../Server/protocol.h"
#include "ServerPacketHandler.h"
#include "BufferReader.h"
#include "BufferWriter.h"

void ServerPacketHandler::HandlePacket(BYTE* buffer, int32 len)
{
	BufferReader br(buffer, len);

	PacketHeader header;
	br.Peek(&header);

	switch (header.id)
	{
	case CS_LOGIN:
		Handle_CS_LOGIN(buffer, len);
		break;
	default:
		cout << "error" << endl;
		break;
	}
}

SendBufferRef ServerPacketHandler::Make_S_TEST(uint64 id, uint32 hp, uint16 attack, vector<BuffData> buffs, wstring name)
{
	SendBufferRef sendBuffer = make_shared<SendBuffer>(4096);
	
	BufferWriter bw(sendBuffer->Buffer(), 4096);

	PacketHeader* header = bw.Reserve<PacketHeader>();

	// id(uint64) 체력(uint32) 공격력(uint16)
	// 가변적
	bw << id << hp << attack;

	// 가변 데이터
	bw << (uint16)buffs.size();

	for (BuffData& buff : buffs)
	{
		bw << buff.buffid << buff.femainTime;
	}

	bw << (uint16)name.size();
	bw.Write((void*)name.data(), name.size() * sizeof(WCHAR));

	header->size = bw.WriteSize();
	header->id = S_TEST;	// 1 : hello message

	sendBuffer->CopyData(bw.GetBuffer(), bw.WriteSize());
	//Send(sendBuffer);

	return sendBuffer;
}

void ServerPacketHandler::Handle_CS_LOGIN(BYTE* buffer, int32 len)
{
	BufferReader br(buffer, len);

	PacketHeader header;
	br >> header;

	wstring name;
	uint16 nameLen;
	br >> nameLen;
	name.resize(nameLen);

	br.Read((void*)name.data(), nameLen * sizeof(WCHAR));

	SendBufferRef sendBuffer = ServerPacketHandler::Make_SC_LOGIN(0, L"김우찬");

	wcout.imbue(std::locale("kor"));
	wcout << name << endl;
	//GSessionManager.Broadcast(sendBuffer);

}

SendBufferRef ServerPacketHandler::Make_SC_LOGIN(uint64 id, wstring name)
{
	SendBufferRef sendBuffer = make_shared<SendBuffer>(4096);

	BufferWriter bw(sendBuffer->Buffer(), 4096);

	PacketHeader* header = bw.Reserve<PacketHeader>();

	// id(uint64) 체력(uint32) 공격력(uint16)
	// 가변적
	bw << id;

	// 가변 데이터
	bw << (uint16)name.size();
	bw.Write((void*)name.data(), name.size() * sizeof(WCHAR));

	header->size = bw.WriteSize();
	header->id = SC_LOGIN;	// 1 : hello message

	sendBuffer->CopyData(bw.GetBuffer(), bw.WriteSize());
	//Send(sendBuffer);

	return sendBuffer;
}
