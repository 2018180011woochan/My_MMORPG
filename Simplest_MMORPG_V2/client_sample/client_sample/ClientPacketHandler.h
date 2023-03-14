#pragma once

enum
{
	S_TEST = 1
};

class ClientPacketHandler
{
public:
	static void HandlePacket(BYTE* buffer, int32 len);
	static void Handle_S_TEST(BYTE* buffer, int32 len);

	static SendBufferRef Make_CS_LOGIN(wstring name);

	static void Handle_SC_LOGIN(BYTE* buffer, int32 len);
};

