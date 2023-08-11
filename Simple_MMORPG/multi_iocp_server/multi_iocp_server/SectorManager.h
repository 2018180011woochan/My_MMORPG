#pragma once
#include "Sector.h"
//class Sector;

class SectorManager
{
public:
	SectorManager() {}
	~SectorManager() {}

public:
	void Init();
	void PushSector(int _id);
	void PopSector(int _id);
	void PushPlayerViewList(int _id);
	void PopPlayerViewList(int _id);
	void SetViewListMove(int _playerID, int _sectorID, unordered_set<int>& _new_vl);
	bool isInSector(int _cid, int _nid);

private:
	bool isValidSector(int _SecID);

private:
	const int SectorCount = 8;			// 8 * 8
	const int SectorAllCount = SectorCount * SectorCount;
	const int SectorWidth = W_WIDTH / SectorCount;
	const int SectorHeight = W_HEIGHT / SectorCount;

	array<Sector*, 64> _Sector;
};

extern SectorManager GSectorManager;