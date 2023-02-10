#pragma once
#include "Types.h"

// 기본적으로는 spinLock
// 32비트를 활용, 상위16비트와 하위 16비트는 의미가 다름
//[WWWWWWWW][WWWWWWWW][RRRRRRRR][RRRRRRRR]
// W : LOCK을 획득하고 있는 쓰레드 아이디
// R : 공유해서 사용하고 있는 ReadCount 

// W -> R (0)
// R -> W (X)
class Lock
{
	enum : uint32
	{
		ACQUIRE_TIMEOUT_TICK = 10000,
		MAX_SPIN_COUNT = 5000,
		WRITE_THREAD_MASK = 0xFFFF'0000,	// 상위 16비트를 뽑아오기 위함
		READ_COUNT_MASK = 0x0000'FFFF,		// 하위 16비트를 뽑아오기 위함
		EMPTY_FLAG = 0x0000'0000
	};

public:
	void WriteLock();
	void WriteUnlock();
	void ReadLock();
	void ReadUnlock();

private:
	Atomic<uint32> _lockFlag = EMPTY_FLAG;
	uint16 _writeCount = 0;
};

// LockGuards

class ReadLockGuard
{
public:
	ReadLockGuard(Lock& lock) : _lock(lock) { _lock.ReadLock(); }
	~ReadLockGuard() { _lock.ReadUnlock(); }

private:
	Lock& _lock;
};

class WriteLockGuard
{
public:
	WriteLockGuard(Lock& lock) : _lock(lock) { _lock.WriteLock(); }
	~WriteLockGuard() { _lock.WriteUnlock(); }

private:
	Lock& _lock;
};