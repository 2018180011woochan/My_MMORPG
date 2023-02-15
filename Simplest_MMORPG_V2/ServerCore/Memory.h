#pragma once
#include "Allocator.h"

// 인자의 개수가 가변적으로 변하는 버전
template<typename Type, typename... Args>
Type* xnew(Args&&... args)
{
	Type* memory = static_cast<Type*>(BaseAllocator::Alloc(sizeof(Type)));

	// 메모리가 할당된 상태에서 생성자를 호출하는 문법
	// placement new
	new(memory)Type(forward<Args>(args)...);	// 생성자의 인자가 매번 바뀌므로 args를 이용	

	return memory;
}

template<typename Type>
void xdelete(Type* obj)
{
	obj->~Type();					// 객체의 소멸자를 호출
	BaseAllocator::Release(obj);	// 메모리를 반납
}

