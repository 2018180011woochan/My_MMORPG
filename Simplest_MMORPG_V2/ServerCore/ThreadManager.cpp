#include "pch.h"
#include "ThreadManager.h"
#include "CoreTLS.h"
#include "CoreGlobal.h"

ThreadManager::ThreadManager()
{
	InitTLS();
}

ThreadManager::~ThreadManager()
{
	Join();
}

void ThreadManager::Launch(function<void(void)> callback)
{
	LockGuard guard(_lock);

	_threads.push_back(thread([=]() {
		InitTLS();		// 처음 스레드가 만들어지면 InitTLS먼저 호출
		callback();		// 콜백으로 넘겨준 함수 실행
		DestroyTLS();	// 함수 실행 됐으면 tls를 지워줌
		}));
}

void ThreadManager::Join()
{
	for (thread& t : _threads)
	{
		if (t.joinable())
			t.join();
	}
	_threads.clear();
}

void ThreadManager::InitTLS()
{
	static Atomic<uint32> SThreadId = 1;
	LThreadId = SThreadId.fetch_add(1);	// 스레드 아이디 설정
}

void ThreadManager::DestroyTLS()
{
	// 혹시 InitTLS에서 동적으로 생성되는 것이 있다면 여기서 날림
}
