#pragma once

enum
{
	S_TEST = 1
};

struct BuffData
{
	uint64 buffid;
	float femainTime;
};

class ServerPacketHandler
{
public:
	static void HandlePacket(BYTE* buffer, int32 len);
	static SendBufferRef Make_S_TEST(uint64 id, uint32 hp, uint16 attack, vector<BuffData> , wstring name) ;

	static void Handle_CS_LOGIN(BYTE* buffer, int32 len);

	static SendBufferRef Make_SC_LOGIN(uint64 id, wstring name) ;
};

