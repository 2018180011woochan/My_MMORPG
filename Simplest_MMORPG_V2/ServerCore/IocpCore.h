#pragma once

// ���߿� ����ؼ� �ٸ��ֵ� �������
// ���ǰ� ����� ����
// iocpevent�� �������� ����Ʈ���� ���Ʈ����
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

	// ������ ����� iocp�� ������ִ� �Լ�
	bool Register(IocpObjectRef iocpObject);
	// ��Ŀ ��������� iocp�� �ϰ��� ���� �θ����Ÿ��� �Լ�
	bool Dispatch(uint32 timeoutMs = INFINITE);

private:
	HANDLE _iocpHandle;
};

