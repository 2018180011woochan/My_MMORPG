#pragma once
#include "Object.h"

enum
{
	S_TEST = 1
};

class Object;

class ClientPacketHandler
{
public:
	static void HandlePacket(BYTE* buffer, int32 len);
	static void Handle_S_TEST(BYTE* buffer, int32 len);

	static SendBufferRef Make_CS_LOGIN(wstring name);
	static SendBufferRef Make_CS_MOVE(int id, DIRECTION _direction);

	static void Handle_SC_LOGIN(BYTE* buffer, int32 len);
	static void Handle_SC_MOVE(BYTE* buffer, int32 len);
};

