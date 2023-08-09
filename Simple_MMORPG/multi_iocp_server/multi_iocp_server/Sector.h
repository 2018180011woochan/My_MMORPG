#pragma once

class Sector
{
public:
	Sector() {}
	Sector(int id) { _ID = id; }

public:
	void SectorAddPlayer(int _playerID);
	void SectorSubPlayer(int _playerID);
	unordered_set<int> GetSectorPlayers() { return _Players; }

private:
	int _ID;
	unordered_set<int> _Players;
	mutex _SectorLock;
};

