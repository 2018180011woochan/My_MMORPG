#include "pch.h"
#include "Lock.h"
#include "CoreTLS.h"

void Lock::WriteLock()
{
	// ������ �����尡 �����ϰ� �ִٸ� ������ �����ϰ� ��
	const uint32 lockThreadId = (_lockFlag.load() & WRITE_THREAD_MASK) >> 16;

	if (LThreadId == lockThreadId)
	{
		_writeCount++;
		return;
	}

	// �ƹ��� ���� �� �����ϰ� ���� ���� �� �����ؼ� �������� ��´�
	const int64 beginTick = ::GetTickCount64();
	const uint32 desired = ((LThreadId << 16) & WRITE_THREAD_MASK);
	while (true)
	{
		for (uint32 spinCount = 0; spinCount < MAX_SPIN_COUNT; spinCount++)
		{
			uint32 expected = EMPTY_FLAG;
			if (_lockFlag.compare_exchange_strong(expected, desired))
			{
				_writeCount++;		// writecount�� �����ϴ� ������ writelock�� ��������� ȣ���ϰ� �ϱ� ���ؼ�
				return;
			}
		}

		// �ʹ� ���� �ɷȴ�
		if (::GetTickCount64() - beginTick >= ACQUIRE_TIMEOUT_TICK)
			cout << "lock time out" << endl;

		this_thread::yield();
	}
}

void Lock::WriteUnlock()
{
	// ReadLock �� Ǯ�� ���� WriteUnlock �Ұ���
	if ((_lockFlag.load() & READ_COUNT_MASK) != 0)
		cout << "invalid unlock order" << endl;

	const int32 lockCount = --_writeCount;
	if (lockCount == 0)
		_lockFlag.store(EMPTY_FLAG);
}

void Lock::ReadLock()
{
	// ������ �����尡 �����ϰ� �ִٸ� ������ �����ϰ� ��
	const uint32 lockThreadId = (_lockFlag.load() & WRITE_THREAD_MASK) >> 16;

	if (LThreadId == lockThreadId)
	{
		_lockFlag.fetch_add(1);
		return;
	}

	// �ƹ��� �����ϰ� ���� ���� �� �����ؼ� ���� ī��Ʈ�� �ø���
	const int64 beginTick = ::GetTickCount64();
	while (true)
	{
		for (uint32 spinCount = 0; spinCount < MAX_SPIN_COUNT; spinCount++)
		{
			uint32 expected = (_lockFlag.load() & READ_COUNT_MASK);

			if (_lockFlag.compare_exchange_strong(expected, expected + 1))
				return;
		}

		// �ʹ� ���� �ɷȴ�
		if (::GetTickCount64() - beginTick >= ACQUIRE_TIMEOUT_TICK)
			cout << "lock time out" << endl;

		this_thread::yield();
	}

}

void Lock::ReadUnlock()
{
	if ((_lockFlag.fetch_sub(1) & READ_COUNT_MASK) == 0)
		cout << "multiple_unlock" << endl;
}
