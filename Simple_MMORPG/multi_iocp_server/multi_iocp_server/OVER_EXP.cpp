#include "pch.h"
#include "OVER_EXP.h"

OVER_EXP::OVER_EXP()
{
	_wsabuf.len = BUF_SIZE;
	_wsabuf.buf = _send_buf;
	_comp_type = OP_RECV;
	ZeroMemory(&_over, sizeof(_over));
}

OVER_EXP::OVER_EXP(char* packet)
{
	_wsabuf.len = packet[0];
	_wsabuf.buf = _send_buf;
	ZeroMemory(&_over, sizeof(_over));
	_comp_type = OP_SEND;
	memcpy(_send_buf, packet, packet[0]);
}
