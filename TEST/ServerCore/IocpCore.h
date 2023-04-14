#pragma once

// 나중에 상속해서 다른애들 만들어줌
// 세션과 비슷한 느낌
// iocpevent는 리드인지 라이트인지 억셉트인지
class IocpObject : public enable_shared_from_this<IocpObject>
{
public:
	virtual HANDLE GetHandle() abstract;
	virtual void Dispatch(class IocpEvent* iocpEvent, int32 numOfByte = 0) abstract;
};

class IocpCore
{
public:
	IocpCore();
	~IocpCore();

	HANDLE GetHandle() { return _iocpHandle; }

	// 소켓을 만들면 iocp에 등록해주는 함수
	bool Register(IocpObjectRef iocpObject);
	// 워커 스레드들이 iocp에 일감이 없나 두리번거리는 함수
	bool Dispatch(uint32 timeoutMs = INFINITE);

private:
	HANDLE _iocpHandle;
};

