#pragma once
#include "Allocator.h"

// ������ ������ ���������� ���ϴ� ����
template<typename Type, typename... Args>
Type* xnew(Args&&... args)
{
	Type* memory = static_cast<Type*>(BaseAllocator::Alloc(sizeof(Type)));

	// �޸𸮰� �Ҵ�� ���¿��� �����ڸ� ȣ���ϴ� ����
	// placement new
	new(memory)Type(forward<Args>(args)...);	// �������� ���ڰ� �Ź� �ٲ�Ƿ� args�� �̿�	

	return memory;
}

template<typename Type>
void xdelete(Type* obj)
{
	obj->~Type();					// ��ü�� �Ҹ��ڸ� ȣ��
	BaseAllocator::Release(obj);	// �޸𸮸� �ݳ�
}

