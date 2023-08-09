#include "pch.h"
#include "Sector.h"

void Sector::SectorAddPlayer(int _playerID)
{
	_SectorLock.lock();
	_Players.emplace(_playerID);
	_SectorLock.unlock();
}

void Sector::SectorSubPlayer(int _playerID)
{
	_SectorLock.lock();
	_Players.erase(_playerID);
	_SectorLock.unlock();
}
