#pragma once

class OVER_EXP
{
public:
	WSAOVERLAPPED _over;
	WSABUF _wsabuf;
	char _send_buf[BUF_SIZE];
	COMP_TYPE _comp_type;
	int target_id;

public:
	OVER_EXP();	
	OVER_EXP(char* packet);
};

