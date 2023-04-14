#include "pch.h"
#include "../Server/protocol.h"
#include "ServerPacketHandler.h"
#include "BufferReader.h"
#include "BufferWriter.h"
#include "../Server/GameSessionManager.h"

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
	case CS_MOVE_OBJECT:
		Handle_CS_MOVE(buffer, len);
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

	int id = GSessionManager.GetAcceptedID();

	short x, y;

	br >> x >> y;
	GSessionManager.SetTargetPos(id, x, y);

	SendBufferRef sendBuffer = ServerPacketHandler::Make_SC_LOGIN(id, name, x ,y);

	GSessionManager.Send(id, sendBuffer);

	// TODO
	// 사정거리 안에 들어오면 다른 플레이어들에게 ADD패킷 보내주기 (시야처리)
	// 지금은 그냥 ADD패킷 보내기
	sendBuffer = ServerPacketHandler::Make_SC_ADD(id, name, x, y);
	GSessionManager.Broadcast(sendBuffer);

	// 사정 거리 안의 이미 접속한 플레이어들의 정보도 ADD패킷으로 보내준다
}

void ServerPacketHandler::Handle_CS_MOVE(BYTE* buffer, int32 len)
{
	BufferReader br(buffer, len);

	PacketHeader header;
	br >> header;

	int id;
	int direction;

	br >> id >> direction;
	cout << id << ", " << direction << endl;

	SendBufferRef sendBuffer = ServerPacketHandler::Make_SC_MOVE(id, direction);

	GSessionManager.Broadcast(sendBuffer);
}

SendBufferRef ServerPacketHandler::Make_SC_LOGIN(uint64 id, wstring name, short x, short y)
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
	
	bw << x << y;

	header->size = bw.WriteSize();
	header->id = SC_LOGIN;	

	sendBuffer->CopyData(bw.GetBuffer(), bw.WriteSize());
	//Send(sendBuffer);

	return sendBuffer;
}

SendBufferRef ServerPacketHandler::Make_SC_MOVE(int id, int direction)
{
	SendBufferRef sendBuffer = make_shared<SendBuffer>(4096);

	BufferWriter bw(sendBuffer->Buffer(), 4096);

	PacketHeader* header = bw.Reserve<PacketHeader>();

	// id(uint64) 체력(uint32) 공격력(uint16)
	// 가변적
	bw << id;

	short x = GSessionManager.GetTargetX(id);
	short y = GSessionManager.GetTargetY(id);

	if (x == -1 || y == -1)
		cout << "move error!" << endl;

	switch (direction) {
	case 0: if (y > 0) y--; break;
	case 1: if (y < W_HEIGHT - 1) y++; break;
	case 2: if (x > 0) x--; break;
	case 3: if (x < W_WIDTH - 1) x++; break;
	default: break;
	}
	bw << x << y;

	GSessionManager.SetTargetPos(id, x, y);

	header->size = bw.WriteSize();
	header->id = SC_MOVE_OBJECT;

	sendBuffer->CopyData(bw.GetBuffer(), bw.WriteSize());

	return sendBuffer;
}

SendBufferRef ServerPacketHandler::Make_SC_ADD(int id, wstring name, short x, short y)
{
	SendBufferRef sendBuffer = make_shared<SendBuffer>(4096);

	BufferWriter bw(sendBuffer->Buffer(), 4096);

	PacketHeader* header = bw.Reserve<PacketHeader>();

	bw << id << name << x << y;

	header->size = bw.WriteSize();
	header->id = SC_ADD_OBJECT;

	sendBuffer->CopyData(bw.GetBuffer(), bw.WriteSize());

	return sendBuffer;
}
