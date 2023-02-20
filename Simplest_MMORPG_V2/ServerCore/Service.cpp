#include "pch.h"
#include "Service.h"

Service::Service(ServiceType type, NetAddress address, IocpCoreRef core, SessionFactory factory, int32 maxSessionCount)
	: _type(type), _netAddress(address), _iocpCore(core), _sessionFactory(factory), _maxSessionCount(maxSessionCount)
{
}

void Service::CloseService()
{
}

SessionRef Service::CreateSession()
{
	return SessionRef();
}

void Service::AddSession(SessionRef session)
{
}

void Service::ReleaseSession(SessionRef session)
{
}
