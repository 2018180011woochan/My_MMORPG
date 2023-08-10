#pragma once

class Sector
{
public:
	Sector() {}
	Sector(int id) { _ID = id; }

public:
	void SectorAddObject(int _objectID);
	void SectorSubObject(int _objectID);
	unordered_set<int> GetSectorPlayers() { return _Objects; }

private:
	int _ID;
	unordered_set<int> _Objects;
	mutex _SectorLock;
};

