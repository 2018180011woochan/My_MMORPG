#include "pch.h"
#include "OVER_EXP.h"

OVER_EXP::OVER_EXP()
{
	_WsaBuf.len = BUF_SIZE;
	_WsaBuf.buf = _SendBuf;
	_CompType = OP_RECV;
	ZeroMemory(&_Over, sizeof(_Over));
}

OVER_EXP::OVER_EXP(char* packet)
{
	_WsaBuf.len = packet[0];
	_WsaBuf.buf = _SendBuf;
	ZeroMemory(&_Over, sizeof(_Over));
	_CompType = OP_SEND;
	memcpy(_SendBuf, packet, packet[0]);
}
