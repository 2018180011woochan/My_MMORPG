#pragma once
#ifndef __STRUCT_H__
#define __STRUCT_H__

#include "Enum.h"
#include "protocol.h"

struct OBJ_STAT
{
	int		ID;
	int		DBID;
	short	x, y;
	char	Name[NAME_SIZE];
	int		Race;
	short	Level;
	int		Exp, MaxExp;
	int		HP, MaxHP;
	//SECTOR  Sector;
	//int		Sector;		//�ӽ�
	int		SectorID;
	bool	IsDead;
};

struct BLOCK {
	int     BlockID;
	short	x, y;
	//SECTOR  Sector;
	int		Sector;		// �ӽ�
	int		SectorID;
};


#endif // !__STRUCT_H__

