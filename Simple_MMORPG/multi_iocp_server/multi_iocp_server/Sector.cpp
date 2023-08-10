#include "pch.h"
#include "Sector.h"

void Sector::SectorAddObject(int _playerID)
{
	_SectorLock.lock();
	_Objects.emplace(_playerID);
	_SectorLock.unlock();
}

void Sector::SectorSubObject(int _playerID)
{
	_SectorLock.lock();
	_Objects.erase(_playerID);
	_SectorLock.unlock();
}
