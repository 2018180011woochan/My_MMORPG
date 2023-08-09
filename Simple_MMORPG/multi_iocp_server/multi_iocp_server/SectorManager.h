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

private:
	const int SectorCount = 8;			// 8 * 8
	const int SectorAllCount = SectorCount * SectorCount;
	const int SectorWidth = W_WIDTH / SectorCount;
	const int SectorHeight = W_HEIGHT / SectorCount;

	array<Sector, 64> _Sector;
};

extern SectorManager GSectorManager;