#pragma once

class OVER_EXP
{
public:
	WSAOVERLAPPED _Over;
	WSABUF _WsaBuf;
	char _SendBuf[BUF_SIZE];
	COMP_TYPE _CompType;
	int _TargetID;

public:
	OVER_EXP();	
	OVER_EXP(char* packet);
};

